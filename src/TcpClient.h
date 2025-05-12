#pragma once

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <chrono>

struct ClientMessageLog
{
    std::string messageId;
    uint64_t sentTimestamp;
    uint64_t receivedTimestamp;
    uint64_t roundTripLatency;
};

class TcpClient
{
public:
    TcpClient(boost::asio::io_context& ioContext, const std::string& host, unsigned short port, const std::string& clientId);
    void Start();
    const std::vector<ClientMessageLog>& GetLogs() const;
    void WriteLogsToFile(const std::string& filename) const;


private:
    void Connect();
    void SendMessage();
    void ReadAck();

    std::string clientId;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::ip::tcp::socket socket;
    std::vector<ClientMessageLog> logs;

    std::string lastMessageId;
    std::chrono::steady_clock::time_point sendTime;
    std::array<char, 128> messageBuffer;
    std::array<char, 64> ackBuffer;
};