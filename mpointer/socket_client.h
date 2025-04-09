//
// Created by roarb on 08/04/2025.
//

#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <string>
#include <vector>
#include <mutex>
#include "../protocol/message.h"

// Inclusión específica de Windows
#include <WinSock2.h>
#include <WS2tcpip.h>

class SocketClient {
public:
    SocketClient();
    ~SocketClient();

    bool connect(const std::string& host, int port);
    bool isConnected() const;
    void disconnect();

    // Métodos para enviar solicitudes al servidor y recibir respuestas
    Message sendRequest(const Message& request);

    // Métodos específicos para cada tipo de operación
    int createMemoryBlock(size_t size, const std::string& type);
    bool setMemoryBlock(int id, const std::vector<char>& data);
    std::vector<char> getMemoryBlock(int id);
    bool increaseRefCount(int id);
    bool decreaseRefCount(int id);

private:
    SOCKET socket_fd_;
    bool connected_;
    std::mutex socket_mutex_;
    std::string host_;
    int port_;

    bool sendMessage(const Message& message);
    Message receiveMessage();
    bool tryReconnect();
};

#endif //SOCKET_CLIENT_H