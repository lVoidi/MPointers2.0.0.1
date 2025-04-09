//
// Created by roarb on 08/04/2025.
//

#ifndef MPOINTER_H
#define MPOINTER_H

#include <memory>
#include <string>
#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include "socket_client.h"

template <typename T>
class MPointer {
public:
    // Método de inicialización estática
    static void Init(const std::string& host, int port) {
        if (!client_) {
            client_ = std::make_shared<SocketClient>();
            if (!client_->connect(host, port)) {
                throw std::runtime_error("No se pudo conectar al Memory Manager");
            }
        }
    }

    // Método para crear un nuevo MPointer
    static MPointer<T> New() {
        if (!client_) {
            throw std::runtime_error("MPointer no inicializado. Llame a Init primero.");
        }

        // Solicitar bloque de memoria al servidor
        int id = client_->createMemoryBlock(sizeof(T), typeid(T).name());
        return MPointer<T>(id);
    }

    // Constructor por defecto
    MPointer() : id_(-1) {}

    // Constructor con ID
    explicit MPointer(int id) : id_(id) {}

    // Constructor de copia
    MPointer(const MPointer<T>& other) : id_(other.id_) {
        if (id_ >= 0) {
            client_->increaseRefCount(id_);
        }
    }

    // Destructor
    ~MPointer() {
        if (id_ >= 0) {
            try {
                client_->decreaseRefCount(id_);
            } catch (const std::exception& e) {
                std::cerr << "Error al decrementar referencias: " << e.what() << std::endl;
            }
        }
    }

    // Operador dereferencia (*)
    T& operator*() {
        if (id_ < 0) {
            throw std::runtime_error("Intento de dereferencia de un MPointer nulo");
        }

        // Obtener datos del servidor
        std::vector<char> data = client_->getMemoryBlock(id_);

        if (data.size() != sizeof(T)) {
            throw std::runtime_error("Tamaño de datos incorrecto al dereferencia MPointer");
        }

        // Copiar los datos a un buffer temporal
        if (!cached_data_) {
            cached_data_ = std::make_unique<T>();
        }
        std::memcpy(cached_data_.get(), data.data(), sizeof(T));

        return *cached_data_;
    }

    // Operador de asignación
    MPointer<T>& operator=(const MPointer<T>& other) {
        if (this != std::addressof(other)) {
            // Decrementar referencia al bloque actual
            if (id_ >= 0) {
                client_->decreaseRefCount(id_);
            }

            // Copiar ID del otro puntero
            id_ = other.id_;

            // Incrementar referencia al nuevo bloque
            if (id_ >= 0) {
                client_->increaseRefCount(id_);
            }

            // Limpiar caché de datos
            cached_data_.reset();
        }
        return *this;
    }

    // Operador dirección (&)
    int operator&() const {
        return id_;
    }

    // Método para asignar un valor
    void setValue(const T& value) {
        if (id_ < 0) {
            throw std::runtime_error("Intento de asignar valor a un MPointer nulo");
        }

        // Serializar el valor para enviar al servidor
        std::vector<char> data(sizeof(T));
        std::memcpy(data.data(), &value, sizeof(T));

        // Enviar al servidor
        if (!client_->setMemoryBlock(id_, data)) {
            throw std::runtime_error("Error al asignar valor al MPointer");
        }

        // Actualizar el caché si existe
        if (cached_data_) {
            *cached_data_ = value;
        }
    }

    // Operador de asignación para el tipo T
    T& operator=(const T& value) {
        setValue(value);
        return *(*this);  // Devuelve el valor actual
    }

    // Validar si el puntero es válido
    bool isValid() const {
        return id_ >= 0;
    }

    // Obtener ID interno
    int getId() const {
        return id_;
    }

private:
    int id_;  // ID del bloque de memoria en el servidor
    static std::shared_ptr<SocketClient> client_;
    std::unique_ptr<T> cached_data_;  // Caché local del valor
};

// Inicialización de la variable estática
template <typename T>
std::shared_ptr<SocketClient> MPointer<T>::client_ = nullptr;

#endif //MPOINTER_H