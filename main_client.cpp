#include "TcpClient.h"
#include <boost/asio.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <client_id>" << std::endl;
        return 1;
    }

    std::string clientId = argv[1];

    boost::asio::io_context ioContext;
    TcpClient client(ioContext, "127.0.0.1", 8080, clientId);
    client.Start();
    client.WriteLogsToFile("../logs/client_message_logs.txt");


    return 0;
}
