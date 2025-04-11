//
// Created by roarb on 08/04/2025.
//
// socket_server.cpp
// socket_server.cpp
#include "socket_server.h"
#include "memory_manager.h"
#include "../protocol/message.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <stdexcept>

#pragma comment(lib, "ws2_32.lib")

SocketServer::SocketServer(int port, MemoryManager* memory_manager)
    : port_(port), memory_manager_(memory_manager), running_(false), server_fd_(INVALID_SOCKET) {

    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
    }
}

SocketServer::~SocketServer() {
    stop();
    WSACleanup();
}

void SocketServer::start() {
    running_ = true;

    // Create socket
    server_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd_ == INVALID_SOCKET) {
        throw std::runtime_error("Failed to create socket: " + std::to_string(WSAGetLastError()));
    }

    // Set socket options
    BOOL opt = TRUE;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        throw std::runtime_error("Failed to set socket options: " + std::to_string(WSAGetLastError()));
    }

    // Bind socket to port
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        throw std::runtime_error("Failed to bind socket: " + std::to_string(WSAGetLastError()));
    }

    // Listen for connections
    if (listen(server_fd_, 5) == SOCKET_ERROR) {
        throw std::runtime_error("Failed to listen: " + std::to_string(WSAGetLastError()));
    }

    std::cout << "Socket server listening on port " << port_ << std::endl;

    // Start accepting connections
    accept_thread_ = std::thread(&SocketServer::acceptConnections, this);
}

void SocketServer::stop() {
    if (running_) {
        running_ = false;

        // Close server socket to interrupt accept()
        if (server_fd_ != INVALID_SOCKET) {
            closesocket(server_fd_);
            server_fd_ = INVALID_SOCKET;
        }

        // Wait for accept thread to finish
        if (accept_thread_.joinable()) {
            accept_thread_.join();
        }
    }
}

void SocketServer::acceptConnections() {
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);

    while (running_) {
        SOCKET client_socket = accept(server_fd_, (struct sockaddr*)&client_addr, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            // Check if server is shutting down
            if (!running_) break;

            std::cerr << "Failed to accept client connection: " << WSAGetLastError() << std::endl;
            continue;
        }

        std::cout << "Client connected" << std::endl;

        // Handle client in a separate thread and detach it
        std::thread client_thread(&SocketServer::handleClient, this, client_socket);
        client_thread.detach();
    }
}

void SocketServer::handleClient(int client_socket) {
    try {
        while (running_) {
            // First, receive the message length
            int message_length = 0;
            int recv_result = recv(client_socket, reinterpret_cast<char*>(&message_length), sizeof(int), 0);
            if (recv_result <= 0) {
                if (recv_result == 0) {
                    std::cout << "Client disconnected" << std::endl;
                } else {
                    std::cerr << "Error receiving message length: " << WSAGetLastError() << std::endl;
                }
                break;
            }

            // Then receive the full message
            std::vector<char> buffer(message_length);
            int bytes_received = 0;
            while (bytes_received < message_length) {
                recv_result = recv(client_socket, buffer.data() + bytes_received,
                                  message_length - bytes_received, 0);
                if (recv_result <= 0) {
                    std::cerr << "Error receiving message data: " << WSAGetLastError() << std::endl;
                    break;
                }
                bytes_received += recv_result;
            }

            // Si salimos del bucle anterior por un error (recv_result <= 0), 
            // no debemos continuar procesando el mensaje.
            if (recv_result <= 0) {
                break;
            }

            // Deserialize the message
            Message request = Message::deserialize(buffer);
            std::cout << "[SocketServer] Received message of type: " << static_cast<int>(request.getType()) << " for socket " << client_socket << std::endl;

            // Process the request
            std::cout << "[SocketServer] Processing request for socket " << client_socket << "..." << std::endl;
            Message response = processRequest(request);
            std::cout << "[SocketServer] Request processed for socket " << client_socket << "." << std::endl;

            // Serialize and send the response
            std::cout << "[SocketServer] Serializing response for socket " << client_socket << "..." << std::endl;
            std::vector<char> response_data = response.serialize();
            int response_length = static_cast<int>(response_data.size());
            std::cout << "[SocketServer] Sending response (length: " << response_length << ") to socket " << client_socket << "..." << std::endl;

            // Send response length
            std::cout << "[SocketServer] Sending response length for socket " << client_socket << "..." << std::endl;
            int send_len_result = send(client_socket, reinterpret_cast<char*>(&response_length), sizeof(int), 0);
            if (send_len_result != sizeof(int)) {
                std::cerr << "[SocketServer] Error sending response length: " << WSAGetLastError() << " for socket " << client_socket << std::endl;
                break;
            }
            std::cout << "[SocketServer] Response length sent for socket " << client_socket << "." << std::endl;

            // Send response data
            std::cout << "[SocketServer] Sending response data for socket " << client_socket << "..." << std::endl;
            int send_data_result = send(client_socket, response_data.data(), response_length, 0);
            if (send_data_result != response_length) {
                std::cerr << "[SocketServer] Error sending response data: " << WSAGetLastError() << " for socket " << client_socket << std::endl;
                break;
            }
            std::cout << "[SocketServer] Response data sent for socket " << client_socket << "." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[SocketServer] Exception in handleClient for socket " << client_socket << ": " << e.what() << std::endl;
    }

    std::cout << "[SocketServer] Closing client socket " << client_socket << "..." << std::endl;
    // Close client socket
    closesocket(client_socket);
    std::cout << "[SocketServer] Client socket " << client_socket << " closed." << std::endl;
}

Message SocketServer::processRequest(const Message& request) {
    switch (request.getType()) {
        case MessageType::CREATE: {
            size_t size = request.getSize();
            const std::string& dataType = request.getDataType();

            int id = memory_manager_->create(size, dataType);

            // Prepare response with the ID
            std::vector<char> response_data(sizeof(int));
            std::memcpy(response_data.data(), &id, sizeof(int));

            return Message::response(id != -1, response_data);
        }

        case MessageType::SET: {
            int id = request.getId();
            const std::vector<char>& data = request.getData();

            bool success = memory_manager_->set(id, data.data(), data.size());

            return Message::response(success);
        }

        case MessageType::GET: {
            int id = request.getId();

            // --- Bloqueo para leer tamaño y datos consistentemente ---
            size_t blockSize = 0;
            bool foundBlock = false;
            {
                std::lock_guard<std::recursive_mutex> lock(memory_manager_->memory_mutex_);
                auto it = memory_manager_->blocks_.find(id);
                if (it != memory_manager_->blocks_.end() && it->second.in_use) {
                    blockSize = it->second.size;
                    foundBlock = true;
                }
            } // Mutex liberado aquí
            // --------------------------------------------------------

            if (!foundBlock) {
                // No se encontró el bloque o no está en uso
                std::cerr << "[SocketServer] GET failed for ID " << id << ": Block not found or not in use when checking size." << std::endl;
                return Message::response(false);
            }

            // Ahora reservamos un buffer del tamaño exacto
            std::vector<char> result(blockSize);

            // Obtener los datos del bloque (get adquiere su propio lock)
            bool success = memory_manager_->get(id, result.data(), blockSize);

            // Solo usar los bytes exactos en la respuesta
            if (success) {
                return Message::response(true, result);
            } else {
                // Loguear por qué get() falló si blockSize > 0
                 std::cerr << "[SocketServer] GET failed for ID " << id << ": memory_manager_->get returned false unexpectedly." << std::endl;
                return Message::response(false);
            }
        }

        case MessageType::INCREASE_REF_COUNT: {
            int id = request.getId();
            bool success = memory_manager_->increaseRefCount(id);
            return Message::response(success);
        }

        case MessageType::DECREASE_REF_COUNT: {
            int id = request.getId();
            bool success = memory_manager_->decreaseRefCount(id);
            return Message::response(success);
        }

        default:
            return Message::response(false);
    }
}