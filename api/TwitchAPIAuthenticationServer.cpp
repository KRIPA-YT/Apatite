#include "TwitchAPIAuthenticationServer.h"
#include <spdlog/spdlog.h>

// ToDo: MOVE TO CONFIG
const int maxRetries = 3;

namespace http = boost::beast::http;
namespace ip = boost::asio::ip;
using json = nlohmann::json;

bool TwitchAPIAuthenticationServer::authenticateUser(TokenPair& tokenPair) {
    if (this->validateToken(tokenPair.access)) return true;
    spdlog::info("User access token invalid, generating new one...");
    try {
        tokenPair = this->refreshUserAccessToken(tokenPair.refresh);
        return true;
    } catch (std::exception) {}
    spdlog::info("Refresh token invalid, generating new one...");
    try {
        tokenPair = this->firstAuth(tokenPair.scopes);
        this->authenticateUser(tokenPair);
        return true;
    } catch (std::exception) {}
    spdlog::error("Reauthorization failed!");
    return false;
}

bool TwitchAPIAuthenticationServer::authenticateApp() {
    if (validateToken(Tokens::fetchInstance().appAccess)) {
        return true;
    }
    spdlog::info("App access token invalid, generating new one...");
    cpr::Response response = cpr::Post(cpr::Url{ "https://id.twitch.tv/oauth2/token" },
        cpr::Payload{
            {"client_id", Tokens::fetchInstance().clientId},
            {"client_secret", Tokens::fetchInstance().clientSecret},
            {"grant_type", "client_credentials"}
        });
    if (response.status_code != 200) {
        return false;
    }
    json responseJson = json::parse(response.text);
    Tokens::fetchInstance().appAccess = responseJson["access_token"].get<std::string>();
    return this->authenticateApp();
}

bool TwitchAPIAuthenticationServer::validateToken(std::string token) {
    cpr::Response response = cpr::Get(cpr::Url{ "https://id.twitch.tv/oauth2/validate" }, cpr::Header{{"Authorization", "Bearer " + token}});
    if (response.status_code != 200) return false;

    json responseJson = json::parse(response.text);
    return Tokens::fetchInstance().clientId == responseJson["client_id"].get<std::string>();
}

void TwitchAPIAuthenticationServer::generateState(const std::vector<std::string> scopes) {
    std::random_device randomDevice;
    std::mt19937 mersenneTwister(randomDevice());
    std::uniform_int_distribution<uint64_t> stateGenerator(0, pow(2, 64));
    this->state = std::format("{:x}", stateGenerator(mersenneTwister));

    std::string scopeString = "";
    for (std::string s : scopes) {
        scopeString.append(s);
        scopeString.append(" ");
    }
    scopeString.pop_back();
    const auto encodedScopesC = curl_easy_escape(nullptr, scopeString.c_str(), static_cast<int>(scopeString.length()));
    std::string encodedScopes(encodedScopesC);
    curl_free(encodedScopesC);

    spdlog::info("https://id.twitch.tv/oauth2/authorize"
        "?response_type=code"
        "&client_id={}"
        "&redirect_uri=http://localhost:3000"
        "&scope={}"
        "&state={}", Tokens::fetchInstance().clientId, encodedScopes, this->state);
}

bool TwitchAPIAuthenticationServer::handleRequest(http::request<http::string_body>& request, ip::tcp::socket& socket) {
    // Prepare the response message
    http::response<http::string_body> response;
    response.version(request.version());
    response.body() = "Authenticating...";
    response.result(http::status::ok);
    response.set(http::field::server, "My HTTP Server");
    response.set(http::field::content_type, "text/plain");
    response.prepare_payload();

    // Get the referer 
    boost::core::basic_string_view<char> referer = request["Referer"];

    if (!referer.contains("code")) {
        // Send the response to the client
        http::write(socket, response);
        return false;
    }
    auto httpHeaders = this->parseHttpHeaders(referer);

    if (this->state != httpHeaders["state"]) {
        // Send the response to the client
        http::write(socket, response);
        return false;
    }

    // Check if authentication flow errored out
    if (httpHeaders.find("error") != httpHeaders.end()) {
        response.body() = "Authentication errored: " + httpHeaders["error_description"];
    }

    response.body() = "Successfully authenticated!";
    this->code = httpHeaders["code"];

    // Send the response to the client
    http::write(socket, response);

    return true;
}

std::map<std::string, std::string> TwitchAPIAuthenticationServer::parseHttpHeaders(boost::core::basic_string_view<char>& request) {
    std::map<std::string, std::string> headers;
    std::string requestString = request;
    std::regex headerRegex("(?:[?&])([^=&]+)=([^&]*)");
    std::smatch result;
    std::string::const_iterator searchStart(requestString.cbegin());
    while (std::regex_search(searchStart, requestString.cend(), result, headerRegex)) {
        std::stringstream stringStream(result.str());
        std::string token;
        std::string headerPair[2];
        for (int i = 0; std::getline(stringStream, token, '='); i++) {
            headerPair[i] = token;
        }
        headers[headerPair[0].substr(1, headerPair[0].length())] = headerPair[1];
        searchStart = result.suffix().first;
    }
    return headers;
}

void TwitchAPIAuthenticationServer::refererCatcher() {
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, { tcp::v4(), 3000 });

    bool authenticated = false;
    while (!authenticated) {
        tcp::socket socket(io_context);
        acceptor.accept(socket);

        // Read the HTTP request
        boost::beast::flat_buffer buffer;
        http::request<http::string_body> request;
        try {
            http::read(socket, buffer, request);
        } catch (const std::exception& exc) {
            std::cerr << exc.what();
        }

        // Handle the request
        authenticated = this->handleRequest(request, socket);

        // Close the socket
        socket.shutdown(tcp::socket::shutdown_send);
    }
}

TokenPair TwitchAPIAuthenticationServer::generateTokens() {
    cpr::Response response = cpr::Post(
        cpr::Url {"https://id.twitch.tv/oauth2/token"},
        cpr::Payload {
            {"client_id", Tokens::fetchInstance().clientId},
            {"client_secret", Tokens::fetchInstance().clientSecret},
            {"code", this->code},
            {"grant_type", "authorization_code"},
            {"redirect_uri", "http://localhost:3000"}
        }
    );
    if (response.status_code != 200) {
        throw std::exception("Couldn't generate access token!");
    }
    json responseJson = json::parse(response.text);
    return TokenPair{
        .access = responseJson["access_token"].get<std::string>(),
        .refresh = responseJson["refresh_token"].get<std::string>(),
        .scopes = responseJson["scope"].get<std::vector<std::string>>()
    };
}

TokenPair TwitchAPIAuthenticationServer::firstAuth(const std::vector<std::string> scopes) {
    this->generateState(scopes);
    this->refererCatcher();
    for (int i = 0; i < maxRetries; i++) {
        try {
            return this->generateTokens();
        } catch (std::exception) {}
        spdlog::error("Refresh token generation failed, Try #{:d}", i);
    }
    throw std::exception("Refresh token generation failed!");
}

TokenPair TwitchAPIAuthenticationServer::refreshUserAccessToken(std::string refresh) {
    cpr::Response response = cpr::Post(cpr::Url{ "https://id.twitch.tv/oauth2/token" },
        cpr::Payload{
        {"client_id", Tokens::fetchInstance().clientId},
        {"client_secret", Tokens::fetchInstance().clientSecret},
        {"grant_type", "refresh_token"},
        {"refresh_token", refresh}
        });
    if (response.status_code == 400 || response.status_code == 401) {
        spdlog::info("Refresh tokens invalid, regenerating...");
        throw std::exception("Invalid refresh token!");
    }
    json responseJson = json::parse(response.text);
    return TokenPair{
        .access = responseJson["access_token"].get<std::string>(),
        .refresh = responseJson["refresh_token"].get<std::string>(),
        .scopes = responseJson["scope"].get<std::vector<std::string>>()
    };
}