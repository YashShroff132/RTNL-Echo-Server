#include "ClientSession.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <unordered_map>

namespace fs = std::filesystem;

static std::ofstream server_log("../logs/server.log", std::ios::app);
static std::unordered_map<std::string, int> clientMessageCount;

ClientSession::ClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    : socket(socket)
{
}

void ClientSession::Start()
{
    Read();
}

void ClientSession::Read()
{
    auto self = shared_from_this();
    socket->async_read_some(boost::asio::buffer(buffer), // not using read it will only for 128 bytes for that use only async_read
        [this, self](const boost::system::error_code& ec, std::size_t bytesTransferred)
        {
            if (!ec)
            {
                HandleMessage(bytesTransferred);
                Read();
            }
            else
            {
                std::cerr << "Read error: " << ec.message() << std::endl;
            }
        });
}

void ClientSession::HandleMessage(std::size_t bytesTransferred)
{
    if (bytesTransferred < 128)
    {
        std::cerr << "Incomplete message received." << std::endl;
        return;
    }

    std::string clientId(buffer.data(), 36);
    uint64_t timestamp;
    std::memcpy(&timestamp, buffer.data() + 36, sizeof(uint64_t));
    std::string payload(buffer.data() + 44, 84);

    Message msg { clientId, timestamp, payload };
    messageHistory.push_back(msg);

    int order = ++clientMessageCount[clientId];

    server_log << "Client ID: " << clientId << "\n";
    server_log << "Timestamp (ns): " << timestamp << "\n";
    server_log << "Payload: " << payload << "\n";
    server_log << "Order: " << order << "\n";
    server_log << "ACK Sent: ACK:" << clientId << "\n---\n";
    server_log.flush();


    SendAck(clientId);
}

void ClientSession::SendAck(const std::string& messageId)
{
    auto self = shared_from_this();
    std::string ack = "ACK:" + messageId; //message id is the client id
    auto ackBuffer = std::make_shared<std::string>(ack);

    boost::asio::async_write(*socket, boost::asio::buffer(*ackBuffer),
        [this, self, ackBuffer](const boost::system::error_code& ec, std::size_t)
        {
            if (!ec)
            {
                std::cout << "Sent ACK for message from [" << *ackBuffer << "]" << std::endl;
            }
            else
            {
                std::cerr << "ACK send error: " << ec.message() << std::endl;
            }
        });
}