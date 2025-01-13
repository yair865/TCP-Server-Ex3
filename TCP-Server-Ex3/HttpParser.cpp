#include "HttpParser.h"

std::string HttpParser::extractBody(const std::string& request) {
    size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart != std::string::npos) {
        bodyStart += 4;  // Skip the "\r\n\r\n" part
        return request.substr(bodyStart);
    }
    return "";
}

RequestType HttpParser::extractMethodType(const std::string& request)
{
    // Extract the first word of the request (the method type)
    size_t methodEnd = request.find(' '); // Find the first space
    if (methodEnd == std::string::npos)
    {
        return RequestType::INVALID; // Invalid request if no space is found
    }

    std::string method = request.substr(0, methodEnd); // Extract the method substring


    if (method == "GET")
    {
        return RequestType::GET;
    }
    else if (method == "POST")
    {
        return RequestType::POST;
    }
    else if (method == "PUT")
    {
        return RequestType::PUT;
    }
    else if (method == "DELETE")
    {
        return RequestType::DEL;
    }
    else if (method == "HEAD")
    {
        return RequestType::HEAD;
    }
    else if (method == "OPTIONS")
    {
        return RequestType::OPTIONS;
    }
    else if (method == "TRACE")
    {
        return RequestType::TRACE;
    }

    return RequestType::INVALID; // Return INVALID if no match is found
}

std::string HttpParser::extractHeader(const std::string& request, const std::string& header) {
    size_t headerPos = request.find(header);  // Find the start of the header
    if (headerPos != std::string::npos) {
        headerPos += header.length();  // Skip past the header name

        // Skip any whitespace after the colon
        while (headerPos < request.length() && std::isspace(request[headerPos])) {
            headerPos++;
        }

        // Find the end of the header value (typically ends with \r\n)
        size_t endOfHeader = request.find("\r\n", headerPos);

        if (endOfHeader != std::string::npos) {
            return request.substr(headerPos, endOfHeader - headerPos);  // Extract the value
        }
    }
    return "";  // Return empty string if header not found
}

std::string HttpParser::extractQueryParam(const std::string& request, const std::string& param) {
    size_t paramPos = request.find(param + "=");  // Find the parameter and "="
    if (paramPos != std::string::npos) {
        paramPos += param.length() + 1;  // Skip past "param="

        size_t paramEnd = request.find(" ", paramPos);  

        // If no "&" is found, we are at the end of the query string
        if (paramEnd == std::string::npos) {
            return request.substr(paramPos);  // Return the value of the last parameter
        }
        else {
            return request.substr(paramPos, paramEnd - paramPos);  // Return the value between "=" and "&"
        }
    }

    return "";  // Return empty string if the parameter is not found
}


std::string HttpParser::extractResource(const std::string& request) {
    // Find the first space after the HTTP method (GET/POST/etc.)
    size_t resourceStart = request.find(" ");
    if (resourceStart != std::string::npos) {
        resourceStart++;  // Skip past the method (GET/POST/etc.)

        // Find the next space, which separates the resource from the HTTP version
        size_t resourceEnd = request.find(" ", resourceStart);
        if (resourceEnd != std::string::npos) {
            // Now check if there is a query parameter (?)
            size_t queryPos = request.find("?", resourceStart);
            if (queryPos != std::string::npos && queryPos < resourceEnd) {
                // Extract the resource part before the query string
                return request.substr(resourceStart, queryPos - resourceStart);
            }
            else {
                // No query parameter, so extract the entire resource until the HTTP version
                return request.substr(resourceStart, resourceEnd - resourceStart);
            }
        }
    }
    return "";  // Return an empty string if resource isn't found
}


int HttpParser::extractLen(const std::string& request) {
    const std::string contentLengthHeader = "Content-Length: ";

    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {

        return 0;
    }

    headerEnd += 4;

    size_t contentLengthPos = request.find(contentLengthHeader);
    if (contentLengthPos == std::string::npos || contentLengthPos > headerEnd) {

        return headerEnd;
    }
    contentLengthPos += contentLengthHeader.length();

    size_t contentLengthEnd = request.find("\r\n", contentLengthPos);
    if (contentLengthEnd == std::string::npos) {

        return headerEnd;
    }
    std::string lengthStr = request.substr(contentLengthPos, contentLengthEnd - contentLengthPos);

    int contentLength = std::stoi(lengthStr);

    return headerEnd + contentLength;
}

HttpParser::HttpParser()
{
}

HttpParser::~HttpParser()
{
}
