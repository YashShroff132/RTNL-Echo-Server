#include "epoll_server.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    unsigned short port = static_cast<unsigned short>(std::stoi(argv[1]));
    EpollServer server(port);
    server.Run();

    return 0;
}