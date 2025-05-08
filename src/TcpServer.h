#pragma once

#include <boost/asio.hpp>
#include <unordered_map>
#include <memory>
#include "ClientSession.h"

class TcpServer
{
public:
    TcpServer(boost::asio::io_context& ioContext, unsigned short port);
    void Start();

private:
    void AcceptConnection();

    boost::asio::ip::tcp::acceptor acceptor;
    std::unordered_map<std::string, std::shared_ptr<ClientSession>> clients;
};