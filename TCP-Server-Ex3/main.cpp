#include "server.h"
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
