#include "TcpServer.h"
#include <iostream>

TcpServer::TcpServer(boost::asio::io_context& ioContext, unsigned short port)
    : acceptor(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}

void TcpServer::Start()
{
    AcceptConnection();
}

void TcpServer::AcceptConnection()
{
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(acceptor.get_executor());

    acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& ec)
    {
        if (!ec)
        {
            std::cout << "New connection accepted." << std::endl;
            auto session = std::make_shared<ClientSession>(socket);
            session->Start();
            std::string clientId = std::to_string(reinterpret_cast<std::uintptr_t>(session.get()));
            clients[clientId] = session;
        }
        AcceptConnection();
    });
}