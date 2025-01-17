#include "HttpParser.h"

using namespace std;

string HttpParser::extractBody(const string& request) {
    size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart != string::npos) {
        bodyStart += 4;
        return request.substr(bodyStart);
    }
    return "";
}

RequestType HttpParser::extractMethodType(const string& request)
{
    size_t methodEnd = request.find(' '); 
    if (methodEnd == string::npos)
    {
        return RequestType::INVALID; 
    }

    string method = request.substr(0, methodEnd); 


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

    return RequestType::INVALID;
}

string HttpParser::extractSpecificHeader(const string& request, const string& header) {
    size_t headerPos = request.find(header);  
    if (headerPos != string::npos) {
        headerPos += header.length();  

        while (headerPos < request.length() && isspace(request[headerPos])) {
            headerPos++;
        }

        size_t endOfHeader = request.find("\r\n", headerPos);

        if (endOfHeader != string::npos) {
            return request.substr(headerPos, endOfHeader - headerPos);
        }
    }
    return "";  
}

string HttpParser::extractHeaders(const string& request) {
    size_t headerStart = request.find("\r\n"); 
    if (headerStart == string::npos) {
        return ""; 
    }
    headerStart += 2; 

    size_t bodyStart = request.find("\r\n\r\n", headerStart);
    if (bodyStart == string::npos) {
        bodyStart = request.length();
    }

    return request.substr(headerStart, bodyStart - headerStart);
}

string HttpParser::extractQueryParam(const string& request, const string& param) {
    size_t paramPos = request.find(param + "=");
    if (paramPos != string::npos) {
        paramPos += param.length() + 1; 

        size_t paramEnd = request.find(" ", paramPos);  

        if (paramEnd == string::npos) {
            return request.substr(paramPos); 
        }
        else {
            return request.substr(paramPos, paramEnd - paramPos);
        }
    }

    return "";
}


string HttpParser::extractResource(const string& request) {
    size_t resourceStart = request.find(" ");
    if (resourceStart != string::npos) {
        resourceStart++;

        size_t resourceEnd = request.find(" ", resourceStart);
        if (resourceEnd != string::npos) {
            size_t queryPos = request.find("?", resourceStart);
            if (queryPos != string::npos && queryPos < resourceEnd) {
                return request.substr(resourceStart, queryPos - resourceStart);
            } else {
                return request.substr(resourceStart, resourceEnd - resourceStart);
            }
        }
    }
    return "";
}


int HttpParser::extractLen(const string& request) {
    const string contentLengthHeader = "Content-Length: ";

    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == string::npos) {

        return 0;
    }

    headerEnd += 4;

    size_t contentLengthPos = request.find(contentLengthHeader);
    if (contentLengthPos == string::npos || contentLengthPos > headerEnd) {

        return headerEnd;
    }
    contentLengthPos += contentLengthHeader.length();

    size_t contentLengthEnd = request.find("\r\n", contentLengthPos);
    if (contentLengthEnd == string::npos) {

        return headerEnd;
    }
    string lengthStr = request.substr(contentLengthPos, contentLengthEnd - contentLengthPos);

    int contentLength = stoi(lengthStr);

    return headerEnd + contentLength;
}

string HttpParser::extractRequestLine(const string& request)
{
    size_t requestLineEnd = request.find("\r\n");
    if (requestLineEnd != string::npos)
    {
        return request.substr(0, requestLineEnd);
    }
    return "";
}

HttpParser::HttpParser()
{
}

HttpParser::~HttpParser()
{
}
