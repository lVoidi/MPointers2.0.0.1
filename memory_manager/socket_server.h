//
// Created by roarb on 08/04/2025.
//

#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#endif //SOCKET_SERVER_H

class SocketServer {
public:
    SocketServer(int port, MemoryManager* memory_manager);
    ~SocketServer();

    void start();
    void stop();

private:
    int server_fd_;
    int port_;
    MemoryManager* memory_manager_;
    bool running_;
    std::vector<std::thread> client_threads_;

    void acceptConnections();
    void handleClient(int client_socket);
    void processMessage(const Message& message, int client_socket);
};