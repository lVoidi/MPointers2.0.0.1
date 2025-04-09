//
// Created by roarb on 08/04/2025.
//

#ifndef MESSAGE_H
#define MESSAGE_H

#endif //MESSAGE_H

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

    // Getters for message properties
    MessageType getType() const;
    // Other accessor methods

private:
    MessageType type_;
    // Message data fields
};