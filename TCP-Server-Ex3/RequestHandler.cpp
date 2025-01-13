#include "RequestHandler.h"
#include <cstring>

RequestHandler::RequestHandler()
{
    path = nullptr;
    body = nullptr;
}

RequestHandler::~RequestHandler()
{
    delete[] path;
    delete[] body;
}

void RequestHandler::handleRequest(RequestType method, const std::string& request, char* response)
{
    switch (method) {
    case RequestType::GET:
        handleGET(request, response);
        break;

    case RequestType::POST:
        handlePOST(request, response);
        break;

    case RequestType::PUT:
        handlePUT(request, response);
        break;

    case RequestType::DEL:
        handleDELETE(request, response);
        break;

    case RequestType::HEAD:
        handleHEAD(request, response);
        break;

    case RequestType::OPTIONS:
        handleOPTIONS(request, response);
        break;

    case RequestType::TRACE:
        handleTRACE(request, response);
        break;

    case RequestType::INVALID:
    default:
        handleINVALID(request, response);
        break;
    }
}

void RequestHandler::handleHEAD(const std::string& request, char* response)
{

}

void RequestHandler::handleOPTIONS(const std::string& request, char* response)
{

}

void RequestHandler::handleTRACE(const std::string& request, char* response)
{

}

void RequestHandler::handleINVALID(const std::string& request, char* response)
{

}

void RequestHandler::handlePUT(const std::string& request, char* response)
{
    // Handle PUT request (e.g., update resource)
    std::cout << "Handling PUT request for path: " << path << std::endl;
    generateResponse(HTTP_OK, "PUT request handled successfully", response);
}

void RequestHandler::handleDELETE(const std::string& request, char* response)
{
    // Handle DELETE request (e.g., delete resource)
    std::cout << "Handling DELETE request for path: " << path << std::endl;
    generateResponse(HTTP_OK, "DELETE request handled successfully", response);
}

void RequestHandler::handleGET(const std::string& request, char* response) {
    std::string requestStr(request);

    std::string resource = parser.extractResource(requestStr);

    // Extract the lang query parameter using parser
    std::string langParam = parser.extractQueryParam(requestStr, "lang");
    if (langParam.empty()) {
        langParam = "en";
    }

    std::string langFolder;
    if (langParam == "en") {
        langFolder = "en";
    }
    else if (langParam == "he") {
        langFolder = "he";
    }
    else if (langParam == "fr") {
        langFolder = "fr";
    }
    else {
        generateResponse(HTTP_BAD_REQUEST, "Unsupported language", response);
        return;
    }

    // Construct the full path to the requested resource
    std::string filePath = "C:/temp/" + langFolder + resource;

    // Check if the file exists
    std::ifstream file(filePath);
    if (!file.is_open()) {
        generateResponse(HTTP_NOT_FOUND, "File not found", response);
        return;
    }

    // Read the file content if it exists
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Display content on the console
    std::cout << "File content for " << filePath << ":\n" << content << std::endl;

    // Send the file content in the response
    generateResponse(HTTP_OK, content.c_str(), response);
}


void RequestHandler::handlePOST(const std::string& request, char* response)
{
    std::string requestStr(request);

    // Extract the resource (URL path, e.g., /index.html) using parser
    std::string resource = parser.extractResource(requestStr);
    if (resource.empty())
    {
        generateResponse(HTTP_BAD_REQUEST, "Invalid URL in request", response);
        return;
    }

    std::cout << "File to save: " << resource << std::endl;

    // Extract the body of the POST request using parser
    std::string body = parser.extractBody(requestStr);
    if (body.empty())
    {
        generateResponse(HTTP_BAD_REQUEST, "POST body not found", response);
        return;
    }

    // Extract the Content-Type header using parser
    std::string contentType = parser.extractHeader(requestStr, "Content-Type:");
    if (contentType.empty())
    {
        generateResponse(HTTP_BAD_REQUEST, "Missing Content-Type header", response);
        return;
    }

    // Handle different content types (text/plain, text/html)
    if (contentType == "text/plain" || contentType == "text/html")
    {
        saveToFile(resource, body.c_str());
        generateResponse(HTTP_OK, (contentType + " file saved successfully").c_str() , response);
    }
    else
    {
        generateResponse(HTTP_BAD_REQUEST, "Unsupported content type", response);
    }
}

void RequestHandler::generateResponse(int statusCode, const char* message, char* response)
{
    const char* status = nullptr;
    switch (statusCode) {
    case HTTP_OK: status = "200 OK"; break;
    case HTTP_BAD_REQUEST: status = "400 Bad Request"; break;
    case HTTP_NOT_FOUND: status = "404 Not Found"; break;
    case HTTP_INTERNAL_SERVER_ERROR: status = "500 Internal Server Error"; break;
    default: status = "500 Internal Server Error"; break;
    }

    const char* responseTemplate = "HTTP/1.1 %s\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s";
    size_t responseSize = strlen(status) + strlen(message) + 100;
    snprintf(response, responseSize, responseTemplate, status, strlen(message), message);
}

void RequestHandler::saveToFile(const std::string& filename, const char* content)
{
    std::string filePath = FILE_PATH + filename;

    std::ofstream file(filePath, std::ios::binary);
    if (file.is_open())
    {
        file.write(content, strlen(content));
        file.close();
        std::cout << "File saved successfully to " << filePath << std::endl;
    }
    else
    {
        std::cerr << "Failed to save the file to " << filePath << std::endl;
    }
}



