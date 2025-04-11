//
// Created by roarb on 08/04/2025.
//

#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <winsock2.h> // Include for SOCKET type
#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include <map>
#include "../protocol/message.h"

class MemoryManager;

class SocketServer {
public:
    SocketServer(int port, MemoryManager* memory_manager);
    ~SocketServer();

    void start();
    void stop();

private:
    void acceptConnections();
    void handleClient(int client_socket);
    Message processRequest(const Message& request);

    int port_;
    MemoryManager* memory_manager_;
    std::atomic<bool> running_;
    SOCKET server_fd_;
    std::thread accept_thread_;
};

#endif //SOCKET_SERVER_H