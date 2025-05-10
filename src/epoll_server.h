// epoll_server.h
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <cstdint>

class EpollServer {
public:
    explicit EpollServer(unsigned short port);
    void Run();

private:
    int server_fd;
    int epoll_fd;
    static constexpr int MAX_EVENTS = 10000;

    struct Client {
        int fd;
        std::array<char, 128> buffer;
        std::vector<std::string> message_history;
    };

    std::unordered_map<int, Client> clients;

    void SetupServerSocket(unsigned short port);
    void HandleNewConnection();
    void HandleClientMessage(int client_fd);
    void SendAck(int client_fd, const std::string& client_id);
    void CloseConnection(int client_fd);
};
