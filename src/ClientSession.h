#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <vector>
#include <string>
#include <optional>

struct Message
{
    std::string clientId;
    uint64_t timestamp;
    std::string payload;
};

class ClientSession : public std::enable_shared_from_this<ClientSession>
{
public:
    explicit ClientSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void Start();

private:
    void Read();
    void HandleMessage(std::size_t bytesTransferred);
    void SendAck(const std::string& messageId);
    void HandleRead(std::size_t bytesTransferred);  // Parses header + message chunks
    void HandleMessage(const std::vector<char>& message); // Takes a complete message block


    std::shared_ptr<boost::asio::ip::tcp::socket> socket;
    std::array<char, 128> buffer;
    std::vector<Message> messageHistory;
    std::vector<char> streamBuffer;              // Holds incomplete or multiple TCP fragments
    std::optional<uint32_t> expectedSize;        // Holds parsed message length from 4-byte header

};