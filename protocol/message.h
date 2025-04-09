//
// Created by roarb on 08/04/2025.
//

#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>

enum class MessageType {
    CREATE,
    SET,
    GET,
    INCREASE_REF_COUNT,
    DECREASE_REF_COUNT,
    RESPONSE
};

class Message {
public:
    static Message createRequest(size_t size, const std::string& type);
    static Message setRequest(int id, const std::vector<char>& data);
    static Message getRequest(int id);
    static Message refCountRequest(int id, bool increase);
    static Message response(bool success, const std::vector<char>& data = {});

    std::vector<char> serialize() const;
    static Message deserialize(const std::vector<char>& buffer);

    // Getters para propiedades del mensaje
    MessageType getType() const { return type_; }
    int getId() const { return id_; }
    size_t getSize() const { return size_; }
    const std::string& getDataType() const { return data_type_; }
    bool isSuccess() const { return success_; }
    const std::vector<char>& getData() const { return data_; }

private:
    MessageType type_;
    int id_;                      // ID del bloque de memoria
    size_t size_;                 // Tamaño a reservar (para CREATE)
    std::string data_type_;       // Tipo de datos (para CREATE)
    bool success_;                // Éxito/fracaso (para RESPONSE)
    std::vector<char> data_;      // Datos serializados

    // Constructor privado para uso interno
    Message(MessageType type, int id = -1, size_t size = 0,
            const std::string& dataType = "", bool success = false,
            const std::vector<char>& data = {});
};

#endif //MESSAGE_H