#include "TwitchAPIAuthenticationServer.h"
#include "../Apatite.h" 

// ToDo: MOVE TO CONFIG
const int maxRetries = 3;

namespace http = boost::beast::http;
namespace ip = boost::asio::ip;
using json = nlohmann::json;

TwitchAPIAuthenticationServer::TwitchAPIAuthenticationServer() {
    if (!this->isTokensValid()) {
        std::cout << "Tokens invalid" << std::endl;
        this->firstAuth();
    }
    if (this->isAccessTokenExpired()) {
        std::cout << "Tokens expired" << std::endl;
        this->refreshAccessToken();
    }
}



void TwitchAPIAuthenticationServer::generateState() {
    std::random_device randomDevice;
    std::mt19937 mersenneTwister(randomDevice());
    std::uniform_int_distribution<uint64_t> stateGenerator(0, pow(2, 64));
    this->state = std::format("{:x}", stateGenerator(mersenneTwister));
    std::cout << std::format("https://id.twitch.tv/oauth2/authorize"
        "?response_type=code"
        "&client_id={}"
        "&redirect_uri=http://localhost:3000"
        "&scope=channel%3Abot%20chat%3Aread%20chat%3Aedit" // TODO: Make scopes configurable
        "&state={}", Tokens::fetchInstance().clientId, this->state) << std::endl;
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
        boost::beast::http::write(socket, response);
        return false;
    }
    auto httpHeaders = this->parseHttpHeaders(referer);

    if (this->state != httpHeaders["state"]) {
        // Send the response to the client
        boost::beast::http::write(socket, response);
        return false;
    }

    // Check if authentication flow errored out
    if (httpHeaders.find("error") != httpHeaders.end()) {
        response.body() = "Authentication errored: " + httpHeaders["error_description"];
    }

    response.body() = "Successfully authenticated!";
    this->code = httpHeaders["code"];

    // Send the response to the client
    boost::beast::http::write(socket, response);

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
        boost::beast::http::request<boost::beast::http::string_body> request;
        try {
            boost::beast::http::read(socket, buffer, request);
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
    if (response.status_code == 400) {
        return false;
    }
    json responseJson = json::parse(response.text);
    Tokens tokens = Tokens::fetchInstance();
    tokens.access = responseJson["access_token"];
    tokens.refresh = responseJson["refresh_token"];
    int64_t expiresIn = responseJson["expires_in"];
    int64_t unixNow = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    tokens.accessExpiry = unixNow + expiresIn;
    return true;
}

void TwitchAPIAuthenticationServer::firstAuth() {
    this->generateState();
    this->refererCatcher();
    for (int i = 0; i < maxRetries; i++) {
        if (this->generateTokens()) {
            return;
        }
        std::cout << "Refresh token generation failed, Try #" << i << std::endl;
    }
}

bool TwitchAPIAuthenticationServer::refreshAccessToken() {
    cpr::Response response = cpr::Post(cpr::Url{ "https://id.twitch.tv/oauth2/token" },
        cpr::Payload{
        {"client_id", Apatite::fetchInstance().authConfig->config["client_id"].as<std::string>()},
        {"client_secret", Apatite::fetchInstance().authConfig->config["client_secret"].as<std::string>()},
        {"grant_type", "refresh_token"},
        {"refresh_token", Apatite::fetchInstance().authConfig->config["refresh_token"].as<std::string>()}
        });
    if (response.status_code == 400) {
        return false;
    }
    if (response.status_code == 401) { // Expired refresh token
        std::cout << "Refresh tokens invalid, regenerating..." << std::endl;
        this->refreshAccessToken();
        return true;
    }
    json responseJson = json::parse(response.text);
    Tokens tokens = Tokens::fetchInstance();
    tokens.access = responseJson["access_token"];
    tokens.refresh = responseJson["refresh_token"];
    int64_t expiresIn = responseJson["expires_in"];
    int64_t unixNow = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    tokens.accessExpiry = unixNow + expiresIn;
    return true;
}

bool TwitchAPIAuthenticationServer::isTokensValid() {
    YAML::Node* authConfig = &Apatite::fetchInstance().authConfig->config;
    return (*authConfig)["refresh_token"].Type() != YAML::NodeType::Null
        && (*authConfig)["access_token"].Type() != YAML::NodeType::Null;
}

bool TwitchAPIAuthenticationServer::isAccessTokenExpired() {
    YAML::Node* authConfig = &Apatite::fetchInstance().authConfig->config;
    int64_t unixNow = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return !this->isInteger((*authConfig)["access_token_expiry"].as<std::string>())
        || !((*authConfig)["access_token_expiry"].as<int64_t>() > unixNow);

}

bool TwitchAPIAuthenticationServer::isInteger(std::string str) {
    char* p;
    strtol(str.c_str(), &p, 10);
    return *p == 0;
}