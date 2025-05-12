#include "ClientSession.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <cstring>
#include <arpa/inet.h> // For ntohl

namespace fs = std::filesystem;

static std::ofstream server_log("../logs/server.log", std::ios::app);
static std::unordered_map<std::string, int> clientMessageCount;

ClientSession::ClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    : socket(socket) {}

void ClientSession::Start() {
    Read();
}

void ClientSession::Read() {
    auto self = shared_from_this();
    auto tempBuffer = std::make_shared<std::array<char, 1024>>();

    socket->async_read_some(boost::asio::buffer(*tempBuffer),
        [this, self, tempBuffer](const boost::system::error_code& ec, std::size_t bytesTransferred) {
            if (!ec) {
                streamBuffer.insert(streamBuffer.end(), tempBuffer->data(), tempBuffer->data() + bytesTransferred);
                HandleRead(bytesTransferred);
                Read();
            } else {
                std::cerr << "Read error: " << ec.message() << std::endl;
            }
        });
}

void ClientSession::HandleRead(std::size_t) {
    while (true) {
        if (!expectedSize && streamBuffer.size() >= 4) {
            uint32_t size = 0;
            std::memcpy(&size, streamBuffer.data(), 4);
            expectedSize = ntohl(size);
            streamBuffer.erase(streamBuffer.begin(), streamBuffer.begin() + 4);
        }

        if (expectedSize && streamBuffer.size() >= expectedSize.value()) {
            std::vector<char> message(streamBuffer.begin(), streamBuffer.begin() + expectedSize.value());
            HandleMessage(message);
            streamBuffer.erase(streamBuffer.begin(), streamBuffer.begin() + expectedSize.value());
            expectedSize.reset();
        } else {
            break; // wait for more data
        }
    }
}

void ClientSession::HandleMessage(const std::vector<char>& message) {
    if (message.size() < 128) {
        std::cerr << "Incomplete logical message received." << std::endl;
        return;
    }

    std::string clientId(message.data(), 36);
    uint64_t timestamp;
    std::memcpy(&timestamp, message.data() + 36, sizeof(uint64_t));
    std::string payload(message.data() + 44, 84);

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

void ClientSession::SendAck(const std::string& messageId) {
    auto self = shared_from_this();
    std::string ack = "ACK:" + messageId;
    auto ackBuffer = std::make_shared<std::string>(ack);

    boost::asio::async_write(*socket, boost::asio::buffer(*ackBuffer),
        [this, self, ackBuffer](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                std::cout << "Sent ACK for message from [" << *ackBuffer << "]" << std::endl;
            } else {
                std::cerr << "ACK send error: " << ec.message() << std::endl;
            }
        });
}
