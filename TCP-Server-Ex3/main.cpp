#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Server.h"
#include <iostream>

int main()
{
    Server server;
    if (!server.initialize())
    {
        std::cout << "Server initialization failed.\n";
        return -1;
    }

    server.run();

    return 0;
}
