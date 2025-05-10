#include "TcpClient.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <bits/this_thread_sleep.h>

namespace fs = std::filesystem;
static std::ofstream client_log("../logs/client.log", std::ios::app);

TcpClient::TcpClient(boost::asio::io_context& ioContext, const std::string& host, unsigned short port, const std::string& clientId)
    : clientId(clientId),
      resolver(ioContext),
      socket(ioContext)
{
    auto endpoints = resolver.resolve(host, std::to_string(port));
    boost::asio::connect(socket, endpoints);
}

void TcpClient::Start()
{
    SendMessage();
    ReadAck();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void TcpClient::SendMessage()
{
    lastMessageId = clientId;
    sendTime = std::chrono::steady_clock::now();

    std::memset(messageBuffer.data(), ' ', 128);
    std::memcpy(messageBuffer.data(), clientId.c_str(), std::min(clientId.size(), size_t(36)));

    auto nowNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                     std::chrono::system_clock::now().time_since_epoch()).count();
    std::memcpy(messageBuffer.data() + 36, &nowNs, sizeof(uint64_t));

    std::string payload = "Payload from client " + clientId;
    std::memcpy(messageBuffer.data() + 44, payload.c_str(), std::min(payload.size(), size_t(84)));

    boost::asio::write(socket, boost::asio::buffer(messageBuffer));

    client_log << "Message ID: " << clientId << "\n";
    client_log << "Sent Timestamp (ns): " << nowNs << "\n";
}

void TcpClient::ReadAck()
{
    size_t len = socket.read_some(boost::asio::buffer(ackBuffer));

    auto recvTime = std::chrono::steady_clock::now();
    uint64_t sendNs = std::chrono::duration_cast<std::chrono::nanoseconds>(sendTime.time_since_epoch()).count();
    uint64_t recvNs = std::chrono::duration_cast<std::chrono::nanoseconds>(recvTime.time_since_epoch()).count();

    ClientMessageLog log {
        lastMessageId,
        sendNs,
        recvNs,
        recvNs - sendNs
    };

    logs.push_back(log);

    client_log << "ACK Received: " << std::string(ackBuffer.data(), len) << "\n";
    client_log << "Received Timestamp (ns): " << recvNs << "\n";
    client_log << "RTT (ns): " << log.roundTripLatency << "\n";
    client_log << "---\n";

    std::cout << "Received ACK: " << std::string(ackBuffer.data(), len)
              << ", RTT(ns): " << log.roundTripLatency << std::endl;
}

const std::vector<ClientMessageLog>& TcpClient::GetLogs() const
{
    return logs;
}