// epoll_server.cpp
#include "epoll_server.h"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
static std::ofstream server_log("../logs/server.log", std::ios::app);


EpollServer::EpollServer(unsigned short port) {
    SetupServerSocket(port);
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    event.data.fd = server_fd;
    event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }
}

void EpollServer::SetupServerSocket(unsigned short port) {
    server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Listening on port " << port << std::endl;
    server_log << "Listening on port " << port << std::endl;
    server_log.flush();
}

void EpollServer::Run() {
    struct epoll_event events[MAX_EVENTS];

    while (true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == server_fd) {
                HandleNewConnection();
            } else {
                HandleClientMessage(events[i].data.fd);
            }
        }
    }
}

void EpollServer::HandleNewConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept4(server_fd, (struct sockaddr*)&client_addr, &client_len, SOCK_NONBLOCK);
    if (client_fd == -1) {
        perror("accept");
        return;
    }

    struct epoll_event event;
    event.data.fd = client_fd;
    event.events = EPOLLIN | EPOLLET; // edge-triggered
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
        perror("epoll_ctl: client_fd");
        close(client_fd);
        return;
    }

    clients[client_fd] = Client{client_fd};
    std::cout << "New client connected, fd: " << client_fd << std::endl;
    server_log << "New client connected, fd: " << client_fd << std::endl;
    server_log.flush();

}

void EpollServer::HandleClientMessage(int client_fd) {
    auto& client = clients[client_fd];
    ssize_t bytes = recv(client_fd, client.buffer.data(), client.buffer.size(), 0);

    if (bytes == 0 || bytes == -1) {
        CloseConnection(client_fd);
        return;
    }

    if (bytes < 128) {
        std::cerr << "Incomplete message from fd " << client_fd << std::endl;
        return;
    }

    std::string clientId(client.buffer.data(), 36);
    uint64_t timestamp;
    std::memcpy(&timestamp, client.buffer.data() + 36, sizeof(uint64_t));
    std::string payload(client.buffer.data() + 52, 76);

    client.message_history.push_back(payload);

    //std::cout << "Stored message from [" << clientId << "] at " << timestamp << "ns, payload: [" << payload << "]\n";
    server_log << "Client ID: " << clientId << "\n";
    server_log << "Timestamp (ns): " << timestamp << "\n";
    server_log << "Payload: " << payload << "\n";
    server_log << "Order: " << client.message_history.size() << "\n";
    server_log << "ACK Sent: ACK:" << clientId << "\n---\n";
    server_log.flush();  // Flush after full block

    SendAck(client_fd, clientId);
}

void EpollServer::SendAck(int client_fd, const std::string& client_id) {
    std::string ack = "ACK:" + client_id;
    send(client_fd, ack.c_str(), ack.size(), 0);
    std::cout << "Sent ACK for message from [" << client_id << "]\n";
}

void EpollServer::CloseConnection(int client_fd) {
    std::cout << "Closing connection on fd: " << client_fd << std::endl;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
    close(client_fd);
    clients.erase(client_fd);
    server_log << "Closing connection on fd: " << client_fd << std::endl;
    server_log.flush();
}
