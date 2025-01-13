#include "Server.h"
#include <iostream>
#include <winsock2.h>
#include <time.h>
#include <string.h>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")


const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;


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
    unsigned long flag = 1;
    if (ioctlsocket(id, FIONBIO, &flag) != 0)
    {
        std::cout << "Http Server: Error at ioctlsocket(): " << WSAGetLastError() << std::endl;
    }

    for (int i = 0; i < maxSockets; i++)
    {
        if (sockets[i].recv == EMPTY)
        {
            sockets[i].id = id;
            sockets[i].recv = what;
            sockets[i].send = IDLE;
            sockets[i].bufferLen = 0;
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

    
    if (!addSocket(msgSocket, RECEIVE))
    {
        std::cout << "\t\tToo many connections, dropped!\n";
        closesocket(id);
    }
}

void Server::receiveMessage(int index)
{
    SOCKET msgSocket = sockets[index].id;
    int len = sockets[index].bufferLen;
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
        sockets[index].bufferLen += bytesRecv;

        if (sockets[index].bufferLen > 0)
        {
           std::string bufferStr(sockets[index].buffer);
           sockets[index].requestLen = parser.extractLen(bufferStr);
           sockets[index].request = bufferStr.string::substr(0, sockets[index].requestLen);
           sockets[index].sendSubType = parser.extractMethodType(sockets[index].request);
           memcpy(sockets[index].buffer, &sockets[index].buffer[sockets[index].requestLen], sockets[index].bufferLen - sockets[index].requestLen);
           sockets[index].bufferLen -= sockets[index].requestLen;
           sockets[index].send = SEND;
        }
    }
}

void Server::sendMessage(int index)
{
    int bytesSent = 0;
    char sendBuff[1024] = { 0 };
    requestHandler.handleRequest(sockets[index].sendSubType, sockets[index].request, sendBuff);
    sockets[index].request = "";
    SOCKET msgSocket = sockets[index].id;
    bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
    if (SOCKET_ERROR == bytesSent)
    {
        std::cout << "Http Server: Error at send(): " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "Http Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";
    

    if (sockets[index].bufferLen == 0)
    {
       sockets[index].send = IDLE;
    }
}
