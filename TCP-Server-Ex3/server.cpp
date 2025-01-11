#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "server.h"
#include <iostream>
#include <winsock2.h>
#include <time.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;
const int SEND_TIME = 1;
const int SEND_SECONDS = 2;

Server::Server(int port, int maxSockets)
    : httpPort(port), maxSockets(maxSockets), socketsCount(0)
{
    sockets.resize(maxSockets);
}

Server::~Server()
{
    WSACleanup();
}

bool Server::initialize()
{
    WSADATA wsaData;
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        std::cout << "Http Server: Error at WSAStartup()\n";
        return false;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == listenSocket)
    {
        std::cout << "Http Server: Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    sockaddr_in serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = INADDR_ANY;
    serverService.sin_port = htons(httpPort);

    if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
    {
        std::cout << "Http Server: Error at bind(): " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    if (SOCKET_ERROR == listen(listenSocket, 5))
    {
        std::cout << "Http Server: Error at listen(): " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    addSocket(listenSocket, LISTEN);
    return true;
}

void Server::run()
{
    while (true)
    {
        handleSelect();
    }
}

void Server::handleSelect()
{
    fd_set waitRecv;
    FD_ZERO(&waitRecv);
    for (int i = 0; i < maxSockets; i++)
    {
        if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
            FD_SET(sockets[i].id, &waitRecv);
    }

    fd_set waitSend;
    FD_ZERO(&waitSend);
    for (int i = 0; i < maxSockets; i++)
    {
        if (sockets[i].send == SEND)
            FD_SET(sockets[i].id, &waitSend);
    }

    int nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
    if (nfd == SOCKET_ERROR)
    {
        std::cout << "Http Server: Error at select(): " << WSAGetLastError() << std::endl;
        return;
    }

    for (int i = 0; i < maxSockets && nfd > 0; i++)
    {
        if (FD_ISSET(sockets[i].id, &waitRecv))
        {
            nfd--;
            switch (sockets[i].recv)
            {
            case LISTEN:
                acceptConnection(i);
                break;
            case RECEIVE:
                receiveMessage(i);
                break;
            }
        }
    }

    for (int i = 0; i < maxSockets && nfd > 0; i++)
    {
        if (FD_ISSET(sockets[i].id, &waitSend))
        {
            nfd--;
            switch (sockets[i].send)
            {
            case SEND:
                sendMessage(i);
                break;
            }
        }
    }
}

bool Server::addSocket(SOCKET id, int what)
{
    for (int i = 0; i < maxSockets; i++)
    {
        if (sockets[i].recv == EMPTY)
        {
            sockets[i].id = id;
            sockets[i].recv = what;
            sockets[i].send = IDLE;
            sockets[i].len = 0;
            socketsCount++;
            return true;
        }
    }

    return false;
}

void Server::removeSocket(int index)
{
    sockets[index].recv = EMPTY;
    sockets[index].send = EMPTY;
    socketsCount--;
}

void Server::acceptConnection(int index)
{
    SOCKET id = sockets[index].id;
    sockaddr_in from;
    int fromLen = sizeof(from);

    SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
    if (INVALID_SOCKET == msgSocket)
    {
        std::cout << "Http Server: Error at accept(): " << WSAGetLastError() << std::endl;
        return;
    }
    std::cout << "Http Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << std::endl;

    unsigned long flag = 1;
    if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
    {
        std::cout << "Http Server: Error at ioctlsocket(): " << WSAGetLastError() << std::endl;
    }

    if (!addSocket(msgSocket, RECEIVE))
    {
        std::cout << "\t\tToo many connections, dropped!\n";
        closesocket(id);
    }
}

void Server::receiveMessage(int index)
{
    SOCKET msgSocket = sockets[index].id;
    int len = sockets[index].len;
    int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

    if (SOCKET_ERROR == bytesRecv)
    {
        std::cout << "Http Server: Error at recv(): " << WSAGetLastError() << std::endl;
        closesocket(msgSocket);
        removeSocket(index);
        return;
    }
    if (bytesRecv == 0)
    {
        closesocket(msgSocket);
        removeSocket(index);
        return;
    }
    else
    {
        sockets[index].buffer[len + bytesRecv] = '\0';
        std::cout << "Http Server: Received: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";
        sockets[index].len += bytesRecv;

        if (sockets[index].len > 0)
        {
            // Create an instance of the RequestHandler and pass the buffer and response to it
            char response[1024];
            requestHandler.handleRequest(sockets[index].buffer, response);

            // If the request is finished (end of headers), send the response
            if (strstr(sockets[index].buffer, "\r\n\r\n"))
            {
                sockets[index].send = SEND;
                strcpy(sockets[index].buffer, response); // Fill buffer with the response
                sockets[index].len = (int)strlen(response); // Set the length of the response

                if (strncmp(sockets[index].buffer, "Exit", 4) == 0)
                {
                    closesocket(msgSocket);
                    removeSocket(index);
                    return;
                }
            }
        }
    }
}

void Server::sendMessage(int index)
{
    int bytesSent = 0;
    char sendBuff[1024];

    memset(sendBuff, 0, sizeof(sendBuff));

    SOCKET msgSocket = sockets[index].id;
    bytesSent = send(msgSocket, sockets[index].buffer, (int)strlen(sockets[index].buffer), 0);
    if (SOCKET_ERROR == bytesSent)
    {
        std::cout << "Http Server: Error at send(): " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "Http Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sockets[index].buffer << "\" message.\n";
    sockets[index].send = IDLE;
}
