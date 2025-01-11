#pragma once

#include <unordered_map>
#include <iostream>
#include <fstream>

class RequestHandler
{
public:
    RequestHandler();
    ~RequestHandler();

    // Main function to handle HTTP requests
    void handleRequest(const char* request, char* response);

private:
    // Helper functions to process specific HTTP methods
    void handleGET(const char* request, char* response);
    void handlePOST(const char* request, char* response);
    void handlePUT(const char* request, char* response);
    void handleDELETE(const char* request, char* response);
    void saveToFile(const std::string& filename, const char* content);
    // Helper to generate the response
    void generateResponse(int statusCode, const char* message, char* response);

    char* path;
    std::unordered_map<std::string, std::string> headers;
    char* body;

    // Constants for HTTP status codes and messages
    static const int HTTP_OK = 200;
    static const int HTTP_BAD_REQUEST = 400;
    static const int HTTP_NOT_FOUND = 404;
    static const int HTTP_INTERNAL_SERVER_ERROR = 500;
};
