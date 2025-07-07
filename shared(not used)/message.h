#pragma once
#include <string>

enum class MessageType{
    Text,
    StatusMessage
};

struct Message {
    MessageType type;
    std::string source;
    std::string content;
};