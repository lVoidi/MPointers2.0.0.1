//
// Created by roarb on 08/04/2025.
//

#include "message.h"
#include <iostream>

Message::Message(MessageType type, int id, size_t size,
                const std::string& dataType, bool success,
                const std::vector<char>& data)
    : type_(type), id_(id), size_(size), data_type_(dataType),
      success_(success), data_(data) {}

Message Message::createRequest(size_t size, const std::string& type) {
    return Message(MessageType::CREATE, -1, size, type);
}

Message Message::setRequest(int id, const std::vector<char>& data) {
    return Message(MessageType::SET, id, 0, "", false, data);
}

Message Message::getRequest(int id) {
    return Message(MessageType::GET, id);
}

Message Message::refCountRequest(int id, bool increase) {
    if (increase) {
        return Message(MessageType::INCREASE_REF_COUNT, id);
    } else {
        return Message(MessageType::DECREASE_REF_COUNT, id);
    }
}

Message Message::response(bool success, const std::vector<char>& data) {
    return Message(MessageType::RESPONSE, -1, 0, "", success, data);
}

std::vector<char> Message::serialize() const {
    std::vector<char> buffer;

    // Agregar el tipo de mensaje (4 bytes)
    int type_val = static_cast<int>(type_);
    buffer.insert(buffer.end(), reinterpret_cast<char*>(&type_val),
                  reinterpret_cast<char*>(&type_val) + sizeof(int));

    // Agregar el ID (4 bytes)
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&id_),
                  reinterpret_cast<const char*>(&id_) + sizeof(int));

    // Agregar el tamaño (8 bytes)
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&size_),
                  reinterpret_cast<const char*>(&size_) + sizeof(size_t));

    // Agregar longitud del tipo de datos (4 bytes)
    int type_length = data_type_.size();
    buffer.insert(buffer.end(), reinterpret_cast<char*>(&type_length),
                  reinterpret_cast<char*>(&type_length) + sizeof(int));

    // Agregar el tipo de datos (variable)
    buffer.insert(buffer.end(), data_type_.begin(), data_type_.end());

    // Agregar indicador de éxito (1 byte)
    char success_val = success_ ? 1 : 0;
    buffer.push_back(success_val);

    // Agregar longitud de datos (4 bytes)
    int data_size = data_.size();
    buffer.insert(buffer.end(), reinterpret_cast<char*>(&data_size),
                  reinterpret_cast<char*>(&data_size) + sizeof(int));

    // Agregar datos (variable)
    buffer.insert(buffer.end(), data_.begin(), data_.end());

    return buffer;
}

Message Message::deserialize(const std::vector<char>& buffer) {
    if (buffer.size() < sizeof(int) * 3 + sizeof(size_t) + 1) {
        throw std::runtime_error("Buffer demasiado pequeño para deserializar");
    }

    size_t offset = 0;

    // Leer tipo de mensaje
    int type_val;
    std::memcpy(&type_val, buffer.data() + offset, sizeof(int));
    offset += sizeof(int);
    MessageType type = static_cast<MessageType>(type_val);

    // Leer ID
    int id;
    std::memcpy(&id, buffer.data() + offset, sizeof(int));
    offset += sizeof(int);

    // Leer tamaño
    size_t size;
    std::memcpy(&size, buffer.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    // Leer longitud del tipo de datos
    int type_length;
    std::memcpy(&type_length, buffer.data() + offset, sizeof(int));
    offset += sizeof(int);

    // Leer tipo de datos
    std::string data_type(buffer.data() + offset, buffer.data() + offset + type_length);
    offset += type_length;

    // Leer indicador de éxito
    bool success = buffer[offset] != 0;
    offset += 1;

    // Leer longitud de datos
    int data_size;
    std::memcpy(&data_size, buffer.data() + offset, sizeof(int));
    offset += sizeof(int);

    // Leer datos
    std::vector<char> data;
    if (data_size > 0) {
        data.assign(buffer.data() + offset, buffer.data() + offset + data_size);
    }

    return Message(type, id, size, data_type, success, data);
}