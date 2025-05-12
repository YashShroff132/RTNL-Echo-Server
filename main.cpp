#include "TcpServer.h"
#include <boost/asio.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    unsigned short port = static_cast<unsigned short>(std::stoi(argv[1])); // getting port number from terminal

    boost::asio::io_context ioContext;

    TcpServer server(ioContext, port);
    server.Start();

    std::vector<std::thread> threads;
    unsigned int threadCount = std::thread::hardware_concurrency();
    for (unsigned int i = 0; i < threadCount; ++i) {
        threads.emplace_back([&ioContext]() { ioContext.run(); });
    }
    for (auto& t : threads) {
        t.join();
    }


    return 0;
}