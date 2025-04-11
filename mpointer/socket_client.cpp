//
// Created by roarb on 08/04/2025.
//

#include "socket_client.h"
#include <stdexcept>
#include <iostream>
#include <thread>
#include <chrono>
#include <Windows.h>
#include "../mpointer/mpointer.h"

// Enlazar con la biblioteca de sockets de Windows
// #pragma comment(lib, "Ws2_32.lib") // Ya está en .h

// Definición e inicialización del contador estático
std::atomic<int> SocketClient::instance_count_{0};

// Definición e inicialización del cliente estático compartido
std::shared_ptr<SocketClient> MPointerConnection::client_ = nullptr;

SocketClient::SocketClient() : socket_fd_(INVALID_SOCKET), connected_(false), port_(0) {
    // Incrementar contador y llamar a WSAStartup si es la primera instancia
    if (instance_count_.fetch_add(1) == 0) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
             instance_count_.fetch_sub(1); // Revertir contador si falla
             std::cerr << "WSAStartup falló con error: " << result << std::endl;
             throw std::runtime_error("No se pudo inicializar Winsock");
        }
        // std::cout << "Winsock inicializado por primera instancia de SocketClient." << std::endl;
    }
}

SocketClient::~SocketClient() {
    disconnect();
    // Decrementar contador y llamar a WSACleanup si es la última instancia
    if (instance_count_.fetch_sub(1) == 1) {
         if (WSACleanup() != 0) {
            // Loggear error pero no lanzar excepción desde destructor
            std::cerr << "WSACleanup failed with error: " << WSAGetLastError() << std::endl;
         }
        // std::cout << "Winsock limpiado por última instancia de SocketClient." << std::endl;
    }
}

bool SocketClient::connect(const std::string& host, int port) {
    std::lock_guard<std::mutex> lock(socket_mutex_);

    host_ = host;
    port_ = port;

    if (connected_) {
        disconnect();
    }

    // Crear el socket
    socket_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd_ == INVALID_SOCKET) {
        std::cerr << "Error al crear el socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Configurar dirección del servidor
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Dirección inválida: " << host << std::endl;
        closesocket(socket_fd_);
        socket_fd_ = INVALID_SOCKET;
        return false;
    }

    // Configurar timeout para el socket
    DWORD timeout = 15000;  // 15 segundos en milisegundos
    setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(socket_fd_, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    // Conectar al servidor
    if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Error de conexión: " << WSAGetLastError() << std::endl;
        closesocket(socket_fd_);
        socket_fd_ = INVALID_SOCKET;
        return false;
    }

    connected_ = true;
    return true;
}

bool SocketClient::isConnected() const {
    return connected_;
}

void SocketClient::disconnect() {
    std::lock_guard<std::mutex> lock(socket_mutex_);

    if (socket_fd_ != INVALID_SOCKET) {
        shutdown(socket_fd_, SD_BOTH);
        closesocket(socket_fd_);
        socket_fd_ = INVALID_SOCKET;
    }
    connected_ = false;
}

bool SocketClient::tryReconnect() {
    if (host_.empty() || port_ == 0) {
        return false;
    }

    std::cerr << "Intentando reconectar al servidor..." << std::endl;

    // Cerrar el socket anterior si está abierto
    if (socket_fd_ != INVALID_SOCKET) {
        shutdown(socket_fd_, SD_BOTH);
        closesocket(socket_fd_);
        socket_fd_ = INVALID_SOCKET;
    }
    connected_ = false;

    // Intentar reconectar hasta 3 veces
    for (int attempt = 0; attempt < 3; ++attempt) {
        // Crear el socket
        socket_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket_fd_ == INVALID_SOCKET) {
            std::cerr << "Error al crear el socket: " << WSAGetLastError() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // Configurar dirección del servidor
        struct sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);

        if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Dirección inválida: " << host_ << std::endl;
            closesocket(socket_fd_);
            socket_fd_ = INVALID_SOCKET;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // Configurar timeout para el socket
        DWORD timeout = 15000;  // 15 segundos en milisegundos
        setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        setsockopt(socket_fd_, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

        // Conectar al servidor
        if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cerr << "Error de conexión: " << WSAGetLastError() << std::endl;
            closesocket(socket_fd_);
            socket_fd_ = INVALID_SOCKET;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        connected_ = true;
        std::cerr << "Reconexión exitosa" << std::endl;
        return true;
    }

    std::cerr << "No se pudo reconectar al servidor después de 3 intentos" << std::endl;
    return false;
}

bool SocketClient::sendMessage(const Message& message) {
    if (!connected_) {
        std::cerr << "No conectado al servidor" << std::endl;
        return false;
    }

    std::vector<char> buffer = message.serialize();

    // Enviar longitud del mensaje
    int length = buffer.size();
    if (send(socket_fd_, reinterpret_cast<char*>(&length), sizeof(int), 0) != sizeof(int)) {
        std::cerr << "Error al enviar tamaño del mensaje: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Enviar mensaje
    int total_sent = 0;
    while (total_sent < buffer.size()) {
        int sent = send(socket_fd_, buffer.data() + total_sent,
                      static_cast<int>(buffer.size() - total_sent), 0);
        if (sent == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error == WSAEINTR) continue;  // Interrumpido por señal, volver a intentar
            std::cerr << "Error al enviar el mensaje: " << error << std::endl;
            return false;
        }
        total_sent += sent;
    }

    return true;
}

Message SocketClient::receiveMessage() {
    if (!connected_) {
        throw std::runtime_error("No conectado al servidor");
    }

    // Recibir longitud del mensaje
    int length = 0;
    int bytes_received = recv(socket_fd_, reinterpret_cast<char*>(&length), sizeof(int), 0);

    if (bytes_received != sizeof(int)) {
        if (bytes_received == 0) {
            throw std::runtime_error("Conexión cerrada por el servidor");
        } else {
            throw std::runtime_error("Error al recibir tamaño del mensaje: " +
                                    std::to_string(WSAGetLastError()));
        }
    }

    if (length <= 0 || length > 1024 * 1024) {  // Limitar tamaño a 1MB
        throw std::runtime_error("Longitud de mensaje inválida: " + std::to_string(length));
    }

    // Recibir mensaje
    std::vector<char> buffer(length);
    bytes_received = 0;
    while (bytes_received < length) {
        int result = recv(socket_fd_, buffer.data() + bytes_received,
                        static_cast<int>(length - bytes_received), 0);
        if (result <= 0) {
            if (result == 0) {
                throw std::runtime_error("Conexión cerrada por el servidor durante recepción");
            } else {
                int error = WSAGetLastError();
                if (error == WSAEINTR) {
                    continue;  // Interrumpido por señal, volver a intentar
                } else {
                    throw std::runtime_error("Error al recibir el mensaje: " +
                                          std::to_string(error));
                }
            }
        }
        bytes_received += result;
    }

    return Message::deserialize(buffer);
}

Message SocketClient::sendRequest(const Message& request) {
    bool reconnection_needed = false;
    
    // Primer intento con el mutex bloqueado
    {
        std::lock_guard<std::mutex> lock(socket_mutex_);
        
        if (!connected_) {
            reconnection_needed = true;
        } else {
            // Intentar enviar el mensaje
            bool sent = false;
            try {
                sent = sendMessage(request);
                if (sent) {
                    // Si el envío fue exitoso, intentar recibir la respuesta
                    try {
                        return receiveMessage();
                    } catch (const std::exception& e) {
                        reconnection_needed = true;
                    }
                } else {
                    reconnection_needed = true;
                }
            } catch (const std::exception&) {
                reconnection_needed = true;
            }
        }
    }
    
    // Si se necesita reconexión, hacerlo fuera del bloque del mutex
    if (reconnection_needed) {
        if (!tryReconnect()) {
            throw std::runtime_error("Error al enviar solicitud: no se pudo reconectar");
        }
        
        // Volver a intentar con el mutex bloqueado después de la reconexión
        std::lock_guard<std::mutex> lock(socket_mutex_);
        
        if (!sendMessage(request)) {
            throw std::runtime_error("Error al enviar solicitud después de reconectar");
        }
        
        try {
            return receiveMessage();
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Error al recibir respuesta: ") + e.what());
        }
    }
    
    // Este punto nunca debería alcanzarse, pero por si acaso lanzamos una excepción
    throw std::runtime_error("Error inesperado en la comunicación con el servidor");
}

int SocketClient::createMemoryBlock(size_t size, const std::string& type) {
    Message request = Message::createRequest(size, type);
    Message response = sendRequest(request);

    if (!response.isSuccess()) {
        throw std::runtime_error("Error al crear bloque de memoria");
    }

    // Extraer ID del bloque creado
    if (response.getData().size() != sizeof(int)) {
        throw std::runtime_error("Respuesta inválida del servidor");
    }

    int id;
    std::memcpy(&id, response.getData().data(), sizeof(int));
    return id;
}

bool SocketClient::setMemoryBlock(int id, const std::vector<char>& data) {
    Message request = Message::setRequest(id, data);
    Message response = sendRequest(request);
    return response.isSuccess();
}

std::vector<char> SocketClient::getMemoryBlock(int id) {
    Message request = Message::getRequest(id);
    Message response = sendRequest(request);

    if (!response.isSuccess()) {
        throw std::runtime_error("Error al obtener datos del bloque de memoria");
    }

    return response.getData();
}

bool SocketClient::increaseRefCount(int id) {
    Message request = Message::refCountRequest(id, true);
    Message response = sendRequest(request);
    return response.isSuccess();
}

bool SocketClient::decreaseRefCount(int id) {
    Message request = Message::refCountRequest(id, false);
    Message response = sendRequest(request);
    return response.isSuccess();
}