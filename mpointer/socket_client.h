//
// Created by roarb on 08/04/2025.
//

#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <stdexcept> // Para stdexcept
#include <iostream> // Para cout/cerr

#include "../protocol/message.h"

#pragma comment(lib, "Ws2_32.lib")

class SocketClient {
public:
    SocketClient(); // Quitar WSAStartup
    ~SocketClient(); // Quitar WSACleanup

    bool connect(const std::string& host, int port);
    bool isConnected() const;
    void disconnect();
    Message sendRequest(const Message& request);

    // Métodos específicos para el Memory Manager
    int createMemoryBlock(size_t size, const std::string& type);
    bool setMemoryBlock(int id, const std::vector<char>& data);
    std::vector<char> getMemoryBlock(int id);
    bool increaseRefCount(int id);
    bool decreaseRefCount(int id);

private:
    bool tryReconnect();
    bool sendMessage(const Message& message);
    Message receiveMessage();

    SOCKET socket_fd_;
    std::string host_;
    int port_;
    std::atomic<bool> connected_;
    std::mutex socket_mutex_; // Para proteger acceso multihilo al socket

    // Contador estático para gestionar Winsock
    static std::atomic<int> instance_count_;
};

#endif //SOCKET_CLIENT_H