#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <string>
#include <vector>
#include "RequestHandler.h"

constexpr auto SERVER_PORT = 8080;
constexpr auto MAX_SOCKETS_NUMBER = 60;
constexpr auto MAX_SOCKET_BUFFER_SIZE = 2048;
constexpr auto MAX_BUFFER_SIZE = 2048;
constexpr auto SECONDS_TILL_TIMEOUT = 120;

typedef struct SocketState
{
    SOCKET id;
    int recv;
    int send;
    RequestType sendSubType;
    std::string request;
    int requestLen;
    char buffer[MAX_SOCKET_BUFFER_SIZE];
    int bufferLen;
    time_t lastActivityTime;
}SocketState;

class Server
{
public:
    Server(int port = SERVER_PORT, int maxSockets = MAX_SOCKETS_NUMBER);
    ~Server();

    bool initialize();
    void run();

private:

    bool addSocket(SOCKET id, int what);
    void removeSocket(int index);
    void acceptConnection(int index);
    void receiveMessage(int index);
    void sendMessage(int index);
    void handleSelect();
    void handleTimeouts();

    SOCKET listenSocket;
    int httpPort;
    int maxSockets;
    std::vector<SocketState> sockets;
    int socketsCount;
    RequestHandler requestHandler;
    HttpParser parser;

    static const int EMPTY = 0;
    static const int LISTEN = 1;
    static const int RECEIVE = 2;
    static const int IDLE = 3;
    static const int SEND = 4;
};
