#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <vector>
#include <string>

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

    std::shared_ptr<boost::asio::ip::tcp::socket> socket;
    std::array<char, 128> buffer;
    std::vector<Message> messageHistory;
};