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
#include <vector>
#include <cstring>
#include "socket_client.h"

// Estructura para mantener la conexión estática compartida
struct MPointerConnection {
    static std::shared_ptr<SocketClient> client_;
    
    static void Init(const std::string& host, int port) {
        if (!client_) {
            client_ = std::make_shared<SocketClient>();
            if (!client_->connect(host, port)) {
                client_.reset(); // Limpiar si la conexión falla
                throw std::runtime_error("MPointerConnection: No se pudo conectar al Memory Manager");
            }
            std::cout << "MPointerConnection: Conexión inicializada." << std::endl;
        }
    }
};

template <typename T>
class MPointer {
private:
    // Clase Proxy anidada
    class Proxy {
        int proxy_id_;
    public:
        explicit Proxy(int id) : proxy_id_(id) {}

        // Operador de asignación (para escritura: *ptr = value)
        Proxy& operator=(const T& value) {
            if (proxy_id_ < 0) {
                throw std::runtime_error("Asignación a un MPointer Proxy nulo");
            }
            // Usar el cliente centralizado
            if (!MPointerConnection::client_) {
                 throw std::runtime_error("MPointer (Proxy) no inicializado. Llame a MPointerConnection::Init primero.");
            }

            // Serializar valor y enviar SET
            std::vector<char> data(sizeof(T));
            std::memcpy(data.data(), &value, sizeof(T));
            if (!MPointerConnection::client_->setMemoryBlock(proxy_id_, data)) {
                throw std::runtime_error("Proxy: Error al asignar valor al MPointer");
            }
            return *this;
        }

        // Operador de conversión (para lectura: T x = *ptr)
        operator T() const {
            if (proxy_id_ < 0) {
                throw std::runtime_error("Lectura desde un MPointer Proxy nulo");
            }
             // Usar el cliente centralizado
            if (!MPointerConnection::client_) {
                 throw std::runtime_error("MPointer (Proxy) no inicializado. Llame a MPointerConnection::Init primero.");
            }
            // Obtener datos del servidor (GET)
            std::vector<char> data = MPointerConnection::client_->getMemoryBlock(proxy_id_);
            if (data.size() < sizeof(T)) {
                 throw std::runtime_error("Proxy: Datos insuficientes del servidor");
            }
            T result;
            std::memcpy(&result, data.data(), sizeof(T));
            return result;
        }
    };

public:
    // Método para crear un nuevo MPointer
    static MPointer<T> New() {
         // Usar el cliente centralizado
        if (!MPointerConnection::client_) {
            throw std::runtime_error("MPointer no inicializado. Llame a MPointerConnection::Init primero.");
        }
        // Usar sizeof(T) directamente. Para Node, T es LinkedList<X>::Node,
        // así que el tamaño será correcto (sizeof(Data) + sizeof(int for next_id))
        int id = MPointerConnection::client_->createMemoryBlock(sizeof(T), typeid(T).name());
        return MPointer<T>(id);
    }

    // Constructor por defecto
    MPointer() : id_(-1) {}

    // Constructor con ID
    explicit MPointer(int id) : id_(id) {
        // Incrementar al tomar referencia a un ID existente
        if (id_ >= 0) {
            if (MPointerConnection::client_) {
                 MPointerConnection::client_->increaseRefCount(id_);
            } else {
                 // Opcional: Advertir si se crea antes de Init
                 std::cerr << "Advertencia: MPointer(id) creado antes de inicializar la conexión." << std::endl;
            }
        }
    }

    // Constructor de copia
    MPointer(const MPointer<T>& other) : id_(other.id_) {
        if (id_ >= 0) {
             // Usar el cliente centralizado
            if (MPointerConnection::client_) {
                 MPointerConnection::client_->increaseRefCount(id_);
            } else {
                 // Podríamos lanzar excepción o loguear advertencia si se copia antes de Init
                 std::cerr << "Advertencia: MPointer copiado antes de inicializar la conexión." << std::endl;
            }
        }
    }

    // Destructor
    ~MPointer() {
        if (id_ >= 0) {
            try {
                 // Usar el cliente centralizado
                 if (MPointerConnection::client_) { // Asegurarse que el cliente existe
                     MPointerConnection::client_->decreaseRefCount(id_);
                 } // Si no existe (ej. programa termina), no hacer nada
            } catch (const std::exception& e) {
                // Evitar que excepciones salgan del destructor
                std::cerr << "Error al decrementar referencias en destructor: " << e.what() << std::endl;
            }
        }
    }

    // Operador dereferencia (*) - Devuelve el Proxy
    Proxy operator*() {
        if (id_ < 0) {
            throw std::runtime_error("Intento de dereferencia de un MPointer nulo");
        }
        return Proxy(id_);
    }

    // Operador de asignación (Copia MPointer)
    MPointer<T>& operator=(const MPointer<T>& other) {
        if (this != std::addressof(other)) {
            // Usar el cliente centralizado
            if (MPointerConnection::client_) {
                 int old_id = id_; // Guardar ID antiguo
                 id_ = other.id_; // Copiar ID nuevo PRIMERO
                 
                 // Incrementar referencia del nuevo ID (si es válido)
                 if (id_ >= 0) {
                    MPointerConnection::client_->increaseRefCount(id_);
                 }
                 
                 // Decrementar referencia del antiguo ID (si era válido)
                 if (old_id >= 0) {
                    MPointerConnection::client_->decreaseRefCount(old_id);
                 }
            } else {
                // Comportamiento si Init no se llamó? Copiar ID pero no refs?
                std::cerr << "Advertencia: Asignación de MPointer antes de inicializar la conexión." << std::endl;
                id_ = other.id_; 
            }
        }
        return *this;
    }

    // Operador dirección (&)
    int operator&() const {
        return id_;
    }

    // Validar si el puntero es válido
    bool isValid() const {
        return id_ >= 0;
    }

    // Obtener ID interno
    int getId() const {
        return id_;
    }

    // Libera la "propiedad" de este MPointer sin decrementar la referencia
    void release() {
        id_ = -1;
    }

private:
    int id_;  // ID del bloque de memoria en el servidor
};

#endif //MPOINTER_H