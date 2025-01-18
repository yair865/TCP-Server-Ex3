#include "RequestHandler.h"
using namespace std;

RequestHandler::RequestHandler()
{
}

RequestHandler::~RequestHandler()
{
}

void RequestHandler::handleRequest(RequestType method, const string& request, char* response)
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

string RequestHandler::validateLanguage(const string& langParam, char* response, bool shouldGenerateResponse)
{
    if (langParam.empty()) {
        return "en";
    }

    if (langParam == "en" || langParam == "he" || langParam == "fr") {
        return langParam;
    }

    if (shouldGenerateResponse)
    {
        generateResponse(HTTP_BAD_REQUEST, "Unsupported language", response);
    }

    return "";
}

string RequestHandler::buildFilePath(const string& langFolder, const string& resource)
{
    return  FILE_PATH + langFolder + resource;
}

bool RequestHandler::validateResource(const string& resource, char* response, bool shouldGenerateResponse)
{
    if (resource.empty()) {

        if (shouldGenerateResponse)
        {
            generateResponse(HTTP_BAD_REQUEST, "Invalid URL in request", response);
        }

        return false;
    }
    return true;
}

bool RequestHandler::fileExists(const string& filePath, char* response, bool shouldGenerateResponse)
{
    ifstream file(filePath);
    if (!file.is_open()) {

        if (shouldGenerateResponse)
        {
            generateResponse(HTTP_NOT_FOUND, "File not found", response);
        }
        return false;
    }
    return true;
}

void RequestHandler::handleGET(const string& request, char* response)
{
    string resource = parser.extractResource(request);
    if (!validateResource(resource, response, AUTO_RESPONSE)) return;

    string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response, AUTO_RESPONSE);
    if (langFolder.empty()) return;

    string filePath = buildFilePath(langFolder, resource);
    if (!fileExists(filePath, response, AUTO_RESPONSE)) return;

    stringstream buffer;
    ifstream file(filePath);
    buffer << file.rdbuf();
    file.close();
    generateResponse(HTTP_OK, buffer.str(), response);
}

void RequestHandler::handlePOST(const string& request, char* response)
{
    string resource = parser.extractResource(request);
    if (!validateResource(resource, response, AUTO_RESPONSE)) return;

    string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response, AUTO_RESPONSE);
    if (langFolder.empty()) return;

    string body = parser.extractBody(request);
    if (body.empty()) {
        generateResponse(HTTP_BAD_REQUEST, "POST body not found", response);
        return;
    }

    string contentType = parser.extractSpecificHeader(request, "Content-Type:");
    if (contentType != "text/plain") {
        generateResponse(HTTP_BAD_REQUEST, "Unsupported content type. Only text/plain is supported for POST method.", response);
        return;
    }

    cout << "POST request received for resource: " << resource << "\n";
    cout << "Data: " << body << endl;

    generateResponse(HTTP_OK, "POST data received successfully", response);
}

void RequestHandler::handlePUT(const string& request, char* response)
{
    string resource = parser.extractResource(request);
    if (!validateResource(resource, response, AUTO_RESPONSE)) return;

    string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response, AUTO_RESPONSE);
    if (langFolder.empty()) return;

    string filePath = buildFilePath(langFolder, resource);

    if (!createDirectories(FILE_PATH + langFolder)) {
        generateResponse(HTTP_INTERNAL_SERVER_ERROR, "Failed to create directories", response);
        return;
    }

    string body = parser.extractBody(request);
    if (body.empty()) {
        generateResponse(HTTP_BAD_REQUEST, "PUT request body is empty", response);
        return;
    }

    saveToFile(filePath, body.c_str());

    if (fileExists(filePath, response, MANUAL_RESPONSE)) {
        generateResponse(HTTP_OK, "File created or updated successfully", response);
    }
    else {
        generateResponse(HTTP_INTERNAL_SERVER_ERROR, "Failed to verify file creation", response);
    }
}

void RequestHandler::handleDELETE(const string& request, char* response)
{
    string resource = parser.extractResource(request);
    if (!validateResource(resource, response, AUTO_RESPONSE)) return;

    string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response, AUTO_RESPONSE);
    if (langFolder.empty()) return;

    string filePath = buildFilePath(langFolder, resource);

    if (ifstream(filePath).good()) {
        if (remove(filePath.c_str()) == 0) {
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

void RequestHandler::handleHEAD(const string& request, char* response)
{
    string resource = parser.extractResource(request);
    if (!validateResource(resource, response, MANUAL_RESPONSE))
    {
        generateResponse(HTTP_BAD_REQUEST, "", response, NO_BODY);
        return;
    }

    string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response, MANUAL_RESPONSE);
    if (langFolder.empty())
    {
        generateResponse(HTTP_BAD_REQUEST, "", response, NO_BODY);
        return;
    }

    string filePath = buildFilePath(langFolder, resource);
    if (!fileExists(filePath, response, MANUAL_RESPONSE))
    {
        generateResponse(HTTP_NOT_FOUND, "", response, NO_BODY);
        return;
    }

    stringstream buffer;
    ifstream file(filePath);
    buffer << file.rdbuf();
    file.close();

    generateResponse(HTTP_OK, "", response, NO_BODY, buffer.str().length());
}


void RequestHandler::handleOPTIONS(const string& request, char* response)
{
    string allowedMethods = "Allow: OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE";
    generateResponse(HTTP_OK, allowedMethods.c_str(), response);
}

void RequestHandler::handleTRACE(const string& request, char* response)
{
    string resource = parser.extractResource(request);
    if (!validateResource(resource, response, AUTO_RESPONSE)) return;

    string langFolder = validateLanguage(parser.extractQueryParam(request, "lang"), response, AUTO_RESPONSE);
    if (langFolder.empty()) return;

    string filePath = buildFilePath(langFolder, resource);
    if (!fileExists(filePath, response, AUTO_RESPONSE)) return;

    ostringstream traceResponse;
    traceResponse << parser.extractRequestLine(request) << "\r\n";
    traceResponse << parser.extractHeaders(request);
    generateResponse(HTTP_OK, traceResponse.str().c_str(), response);
}

void RequestHandler::handleINVALID(const string& request, char* response)
{
    generateResponse(HTTP_BAD_REQUEST, "Unsupported Method", response);
}

string RequestHandler::getStatusMessage(int statusCode) {
    switch (statusCode) {
    case HTTP_OK: return "200 OK";
    case HTTP_BAD_REQUEST: return "400 Bad Request";
    case HTTP_NOT_FOUND: return "404 Not Found";
    case HTTP_INTERNAL_SERVER_ERROR: return "500 Internal Server Error";
    default: return "500 Internal Server Error";
    }
}

string RequestHandler::getCurrentTime() {
    time_t now = time(nullptr);
    char dateStr[100];
    strftime(dateStr, sizeof(dateStr), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
    return string(dateStr);
}

string RequestHandler::generateResponseBody(const string& status, const string& message) {
    ostringstream bodyStream;
    bodyStream << "<!DOCTYPE html>\n"
        << "<html>\n"
        << "<head>\n"
        << "    <meta charset=\"UTF-8\">\n"
        << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        << "    <title>" << status << "</title>\n"
        << "</head>\n"
        << "<body>\n"
        << "    <h1>" << status << "</h1>\n"
        << "    <p>" << message << "</p>\n"
        << "</body>\n"
        << "</html>";
    return bodyStream.str();
}

string RequestHandler::buildHttpResponse(const string& status, const string& date, const string& body, size_t contentLength) {
    ostringstream responseStream;
    responseStream << "HTTP/1.1 " << status << "\r\n"
        << "Content-Type: text/html\r\n"
        << "Content-Length: " << contentLength << "\r\n"
        << "Server: HTTP Web Server\r\n"
        << "Date: " << date << "\r\n"
        << "\r\n"
        << body;
    return responseStream.str();
}

void RequestHandler::generateResponse(int statusCode, const string& message, char*& response, bool shouldGenerateBody, size_t contentLength) {
    string status = getStatusMessage(statusCode);
    string date = getCurrentTime();
    string responseBody = (statusCode != HTTP_OK) && shouldGenerateBody ? generateResponseBody(status, message) : message;
    size_t length = (contentLength > 0) ? contentLength : responseBody.length();
    string httpResponse = buildHttpResponse(status, date, responseBody, length);

    strcpy(response, httpResponse.c_str());
}

void RequestHandler::saveToFile(const string& filename, const char* content)
{
    string filePath = filename;

    ofstream file(filePath, ios::binary);
    if (file.is_open())
    {
        file.write(content, strlen(content));
        file.close();
        cout << "File saved successfully to " << filePath << endl;
    }
    else
    {
        cerr << "Failed to save the file to " << filePath << endl;
    }
}

bool RequestHandler::createDirectories(const string& path) {
    wstring widePath(path.begin(), path.end());

    if (CreateDirectory(widePath.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        return true;
    }

    return false;
}