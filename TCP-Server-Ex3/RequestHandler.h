#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <unordered_map>
#include <fstream>    
#include <sstream>     
#include <iostream>    
#include <windows.h> 
#include "HttpParser.h"



class RequestHandler{
public:
    RequestHandler();
    ~RequestHandler();

    // Main function to handle HTTP requests
    void handleRequest(RequestType method,const std::string& request, char* response);

private:
    HttpParser parser;
    // Helper functions to process specific HTTP methods
    void handleGET(const std::string& request, char* response);
    void handlePOST(const std::string& request, char* response);
    void handlePUT(const std::string& request, char* response);
    void handleDELETE(const std::string& request, char* response);
    void handleHEAD(const std::string& request, char* response);
    void handleOPTIONS(const std::string& request, char* response);
    void handleTRACE(const std::string& request, char* response);
    void handleINVALID(const std::string& request, char* response);

    void saveToFile(const std::string& filename, const char* content);

    // Helper to generate the response
    void generateResponse(int statusCode, const char* message, char* response);

    char* path;
    std::unordered_map<std::string, std::string> headers;
    char* body;

    const std::string FILE_PATH = "C:\\temp\\";

    static const int HTTP_OK = 200;
    static const int HTTP_BAD_REQUEST = 400;
    static const int HTTP_NOT_FOUND = 404;
    static const int HTTP_INTERNAL_SERVER_ERROR = 500;
};
