#include "RequestHandler.h"
#include <cstring>

RequestHandler::RequestHandler()
{
}

RequestHandler::~RequestHandler()
{
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

// Helper function: Validate language and return folder
std::string RequestHandler::validateLanguage(const std::string& langParam, char* response)
{
    if (langParam.empty()) {
        return "en";
    }

    if (langParam == "en" || langParam == "he" || langParam == "fr") {
        return langParam;
    }

    generateResponse(HTTP_BAD_REQUEST, "Unsupported language", response);
    return "";
}

// Helper function: Build file path
std::string RequestHandler::buildFilePath(const std::string& langFolder, const std::string& resource)
{
    return  FILE_PATH + langFolder + resource;
}

// Helper function: Check resource existence and respond
bool RequestHandler::validateResource(const std::string& resource, char* response)
{
    if (resource.empty()) {
        generateResponse(HTTP_BAD_REQUEST, "Invalid URL in request", response);
        return false;
    }
    return true;
}

// Helper function: Check file existence
bool RequestHandler::fileExists(const std::string& filePath, char* response)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        generateResponse(HTTP_NOT_FOUND, "File not found", response);
        return false;
    }
    return true;
}

void RequestHandler::handleHEAD(const std::string& request, char* response)
{
    std::string resource = parser.extractResource(request);
    if (!validateResource(resource, response)) return;

    std::string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response);
    if (langFolder.empty()) return;

    std::string filePath = buildFilePath(langFolder, resource);
    if (!fileExists(filePath, response)) return;

    generateResponse(HTTP_OK, "", response);
}

void RequestHandler::handleOPTIONS(const std::string& request, char* response)
{
    std::string resource = parser.extractResource(request);
    if (!validateResource(resource, response)) return;

    std::string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response);
    if (langFolder.empty()) return;

    std::string filePath = buildFilePath(langFolder, resource);
    if (!fileExists(filePath, response)) return;

    std::string allowedMethods = "GET, POST, PUT, DELETE, OPTIONS";
    generateResponse(HTTP_OK, allowedMethods.c_str(), response);
}

void RequestHandler::handleTRACE(const std::string& request, char* response)
{
    std::string resource = parser.extractResource(request);
    if (!validateResource(resource, response)) return;

    std::string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response);
    if (langFolder.empty()) return;

    std::string filePath = buildFilePath(langFolder, resource);
    if (!fileExists(filePath, response)) return;

    std::ostringstream traceResponse;
    traceResponse << parser.extractRequestLine(request) << "\r\n";
    traceResponse << parser.extractHeaders(request);
    generateResponse(HTTP_OK, traceResponse.str().c_str(), response);
}

void RequestHandler::handleINVALID(const std::string& request, char* response)
{
    generateResponse(HTTP_BAD_REQUEST, "Unsupported Method", response);
}

void RequestHandler::handlePUT(const std::string& request, char* response)
{
    std::string resource = parser.extractResource(request);
    if (!validateResource(resource, response)) return;

    std::string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response);
    if (langFolder.empty()) return;

    std::string filePath = buildFilePath(langFolder, resource);

    std::string body = parser.extractBody(request);
    if (body.empty()) {
        generateResponse(HTTP_BAD_REQUEST, "PUT request body is empty", response);
        return;
    }

    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        generateResponse(HTTP_INTERNAL_SERVER_ERROR, "Failed to create or update the resource", response);
        return;
    }

    outFile.write(body.c_str(), body.size());
    outFile.close();

    if (fileExists(filePath, response)) {
        generateResponse(HTTP_OK, "File created or updated successfully", response);
    }
    else {
        generateResponse(HTTP_INTERNAL_SERVER_ERROR, "Failed to verify file creation", response);
    }
}

void RequestHandler::handleDELETE(const std::string& request, char* response)
{
    std::string resource = parser.extractResource(request);
    if (!validateResource(resource, response)) return;

    std::string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response);
    if (langFolder.empty()) return;

    std::string filePath = buildFilePath(langFolder, resource);

    if (std::ifstream(filePath).good()) {
        if (std::remove(filePath.c_str()) == 0) {
            generateResponse(HTTP_OK, "Resource deleted successfully", response);
        }
        else {
            generateResponse(HTTP_INTERNAL_SERVER_ERROR, "Failed to delete the resource", response);
        }
    }
    else {
        generateResponse(HTTP_NOT_FOUND, "Resource not found", response);
    }
}

void RequestHandler::handleGET(const std::string& request, char* response)
{
    std::string resource = parser.extractResource(request);
    if (!validateResource(resource, response)) return;

    std::string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response);
    if (langFolder.empty()) return;

    std::string filePath = buildFilePath(langFolder, resource);
    if (!fileExists(filePath, response)) return;

    std::stringstream buffer;
    std::ifstream file(filePath);
    buffer << file.rdbuf();
    generateResponse(HTTP_OK, buffer.str().c_str(), response);
}

void RequestHandler::handlePOST(const std::string& request, char* response)
{
    std::string resource = parser.extractResource(request);
    if (!validateResource(resource, response)) return;

    std::string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response);
    if (langFolder.empty()) return;

    std::string filePath = buildFilePath(langFolder, resource);

    std::string body = parser.extractBody(request);
    if (body.empty()) {
        generateResponse(HTTP_BAD_REQUEST, "POST body not found", response);
        return;
    }

    std::string contentType = parser.extractSpecificHeader(request , "Content-Type:");
    if (contentType != "text/html") {
        generateResponse(HTTP_BAD_REQUEST, "Unsupported content type. Only text/html is supported.", response);
        return;
    }

    saveToFile(filePath, body.c_str());
    generateResponse(HTTP_OK, "HTML file saved successfully", response);
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

    time_t now = time(0);
    char dateStr[100];
    strftime(dateStr, sizeof(dateStr), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));

    const char* server = "HTTP Web Server";

    const char* responseTemplate = "HTTP/1.1 %s\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Server: %s\r\n"
        "Date: %s\r\n\r\n%s";

    size_t responseSize = strlen(status) + strlen(message) + strlen(server) + strlen(dateStr) + 200;
    snprintf(response, responseSize, responseTemplate, status, strlen(message), server, dateStr, message);
}

void RequestHandler::saveToFile(const std::string& filename, const char* content)
{
    std::string filePath = filename;

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



