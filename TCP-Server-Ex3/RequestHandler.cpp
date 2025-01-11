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

void RequestHandler::handleRequest(const char* request, char* response)
{
    if (strncmp(request, "GET ", 4) == 0) {
        handleGET(request, response);
    }
    else if (strncmp(request, "POST ", 5) == 0) {
        handlePOST(request, response);
    }
    else if (strncmp(request, "PUT ", 4) == 0) {
        handlePUT(request, response);
    }
    else if (strncmp(request, "DELETE ", 7) == 0) {
        handleDELETE(request, response);
    }
    else {
        generateResponse(HTTP_BAD_REQUEST, "Unsupported HTTP Method", response);
    }
}


void RequestHandler::handleGET(const char* request, char* response)
{
    // Handle GET request (e.g., fetch resources, return status)
    std::cout << "Handling GET request for path: " << path << std::endl;
    generateResponse(HTTP_OK, "GET request handled successfully", response);
}

void RequestHandler::handlePOST(const char* request, char* response)
{
    // Extract the body from the request (after "\r\n\r\n")
    const char* body = strstr(request, "\r\n\r\n");
    if (body)
    {
        body += 4; // Skip past "\r\n\r\n"

        // Determine the content type
        const char* contentTypeHeader = strstr(request, "Content-Type:");
        if (contentTypeHeader)
        {
            contentTypeHeader += strlen("Content-Type: ");  // Skip "Content-Type: "
            const char* endOfContentType = strstr(contentTypeHeader, "\r\n");
            std::string contentType(contentTypeHeader, endOfContentType);

            // Handle text content (like "Hello")
            if (contentType == "text/plain")
            {
                // Save as a text file
                saveToFile("received_text.txt", body);
                generateResponse(HTTP_OK, "Text data saved successfully", response);
            }
            // Handle HTML content
            else if (contentType == "text/html")
            {
                // Save as an HTML file
                saveToFile("received_file.html", body);
                generateResponse(HTTP_OK, "HTML file saved successfully", response);
            }
            else
            {
                generateResponse(HTTP_BAD_REQUEST, "Unsupported content type", response);
            }
        }
        else
        {
            generateResponse(HTTP_BAD_REQUEST, "Missing Content-Type header", response);
        }
    }
    else
    {
        generateResponse(HTTP_BAD_REQUEST, "POST body not found", response);
    }
}


void RequestHandler::handlePUT(const char* request, char* response)
{
    // Handle PUT request (e.g., update resource)
    std::cout << "Handling PUT request for path: " << path << std::endl;
    generateResponse(HTTP_OK, "PUT request handled successfully", response);
}

void RequestHandler::handleDELETE(const char* request, char* response)
{
    // Handle DELETE request (e.g., delete resource)
    std::cout << "Handling DELETE request for path: " << path << std::endl;
    generateResponse(HTTP_OK, "DELETE request handled successfully", response);
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

    // Prepare response (simplified, real-world cases may require dynamic memory management)
    const char* responseTemplate = "HTTP/1.1 %s\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s";
    size_t responseSize = strlen(status) + strlen(message) + 100;
    snprintf(response, responseSize, responseTemplate, status, strlen(message), message);
}

void RequestHandler::saveToFile(const std::string& filename, const char* content)
{
    // Open the file in write mode
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open())
    {
        file.write(content, strlen(content));  // Write the content to the file
        file.close();
        std::cout << "File saved: " << filename << std::endl;
    }
    else
    {
        std::cerr << "Failed to save the file: " << filename << std::endl;
    }
}
