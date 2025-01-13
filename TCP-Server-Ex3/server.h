#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <string>
#include <vector>
#include "RequestHandler.h"

class Server
{
public:
    Server(int port = 27015, int maxSockets = 60);
    ~Server();

    bool initialize();
    void run();

private:
    typedef struct SocketState
    {
        SOCKET id;
        int recv;
        int send;
        RequestType sendSubType;
        std::string request;
        int requestLen;
        char buffer[2048];
        int bufferLen;
    }SocketState;

    bool addSocket(SOCKET id, int what);
    void removeSocket(int index);
    void acceptConnection(int index);
    void receiveMessage(int index);
    void sendMessage(int index);

    void handleSelect();

    SOCKET listenSocket;
    int httpPort;
    int maxSockets;
    std::vector<SocketState> sockets;
    int socketsCount;
    RequestHandler requestHandler;
    HttpParser parser;

};
