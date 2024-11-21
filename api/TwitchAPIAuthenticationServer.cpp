#include "TwitchAPIAuthenticationServer.h"
#include "../Apatite.h" 

// ToDo: MOVE TO CONFIG
const int maxRetries = 3;

namespace http = boost::beast::http;
namespace ip = boost::asio::ip;
using json = nlohmann::json;

bool TwitchAPIAuthenticationServer::authenticate() {
    if (!this->authenticateUser()) return false;
    return this->authenticateApp();
}

bool TwitchAPIAuthenticationServer::authenticateUser() {
    if (this->validateToken(Tokens::fetchInstance().userAccess)) return true;
    spdlog::info("User access token invalid, generating new one...");
    if (this->refreshUserAccessToken()) return true;
    spdlog::info("Refresh token invalid, generating new one...");
    if (this->firstAuth()) return this->authenticateUser();

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
    int64_t expiresIn = responseJson["expires_in"].get<int64_t>();
    int64_t unixNow = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    Tokens::fetchInstance().appAccessExpiry = unixNow + expiresIn;
    return this->authenticateApp();
}

bool TwitchAPIAuthenticationServer::validateToken(std::string token) {
    cpr::Response response = cpr::Get(cpr::Url{ "https://id.twitch.tv/oauth2/validate" }, cpr::Header{{"Authorization", "Bearer " + token}});
    if (response.status_code != 200) return false;

    json responseJson = json::parse(response.text);
    return Tokens::fetchInstance().clientId == responseJson["client_id"].get<std::string>();
}

void TwitchAPIAuthenticationServer::generateState() {
    std::random_device randomDevice;
    std::mt19937 mersenneTwister(randomDevice());
    std::uniform_int_distribution<uint64_t> stateGenerator(0, pow(2, 64));
    this->state = std::format("{:x}", stateGenerator(mersenneTwister));
    spdlog::info("https://id.twitch.tv/oauth2/authorize"
        "?response_type=code"
        "&client_id={}"
        "&redirect_uri=http://localhost:3000"
        "&scope=channel%3Abot%20chat%3Aread%20user%3Aread%3Achat%20user%3Abot%20user%3Awrite%3Achat%20chat%3Aedit" // TODO: Make scopes configurable
        "&state={}", Tokens::fetchInstance().clientId, this->state);
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

bool TwitchAPIAuthenticationServer::generateTokens() {
    cpr::Response response = cpr::Post(cpr::Url{ "https://id.twitch.tv/oauth2/token" },
        cpr::Payload{
        {"client_id", Tokens::fetchInstance().clientId},
        {"client_secret", Tokens::fetchInstance().clientSecret},
        {"code", this->code},
        {"grant_type", "authorization_code"},
        {"redirect_uri", "http://localhost:3000"}
        });
    if (response.status_code != 200) {
        return false;
    }
    json responseJson = json::parse(response.text);
    Tokens::fetchInstance().userAccess = responseJson["access_token"].get<std::string>();
    Tokens::fetchInstance().refresh = responseJson["refresh_token"].get<std::string>();
    int64_t expiresIn = responseJson["expires_in"].get<int64_t>();
    int64_t unixNow = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    Tokens::fetchInstance().userAccessExpiry = unixNow + expiresIn;
    return true;
}

bool TwitchAPIAuthenticationServer::firstAuth() {
    this->generateState();
    this->refererCatcher();
    for (int i = 0; i < maxRetries; i++) {
        if (this->generateTokens()) {
            return true;
        }
        spdlog::error("Refresh token generation failed, Try #{:d}", i);
    }
    return false;
}

bool TwitchAPIAuthenticationServer::refreshUserAccessToken() {
    cpr::Response response = cpr::Post(cpr::Url{ "https://id.twitch.tv/oauth2/token" },
        cpr::Payload{
        {"client_id", Tokens::fetchInstance().clientId},
        {"client_secret", Tokens::fetchInstance().clientSecret},
        {"grant_type", "refresh_token"},
        {"refresh_token", Tokens::fetchInstance().refresh}
        });
    if (response.status_code == 400) {
        return false;
    }
    if (response.status_code == 401) { // Expired refresh token
        spdlog::info("Refresh tokens invalid, regenerating...");
        this->refreshUserAccessToken();
        return true;
    }
    json responseJson = json::parse(response.text);
    Tokens::fetchInstance().userAccess = responseJson["access_token"].get<std::string>();
    Tokens::fetchInstance().refresh = responseJson["refresh_token"].get<std::string>();
    int64_t expiresIn = responseJson["expires_in"].get<int64_t>();
    int64_t unixNow = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    Tokens::fetchInstance().userAccessExpiry = unixNow + expiresIn;
    return true;
}

bool TwitchAPIAuthenticationServer::isUserTokensValid() {
    YAML::Node* authConfig = &Apatite::fetchInstance().authConfig->config;
    return (*authConfig)["refresh_token"].Type() != YAML::NodeType::Null
        && (*authConfig)["access_token"].Type() != YAML::NodeType::Null;
}