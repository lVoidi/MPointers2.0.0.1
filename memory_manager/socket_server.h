//
// Created by roarb on 08/04/2025.
//

#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#endif //SOCKET_SERVER_H
#pragma once
// socket_server.h
#pragma once
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include "../protocol/message.h"

class MemoryManager;

class SocketServer {
public:
    SocketServer(int port, MemoryManager* memory_manager);
    ~SocketServer();

    void start();
    void stop();

private:
    unsigned long long server_fd_; // Use SOCKET type which is unsigned long long
    int port_;
    MemoryManager* memory_manager_;
    std::atomic<bool> running_;
    std::thread accept_thread_;
    std::vector<std::thread> client_threads_;

    void acceptConnections();
    void handleClient(int client_socket);
    Message processRequest(const Message& request);
};