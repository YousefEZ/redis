#ifndef INCLUDED_VARIABLE_ENCODER_H
#define INCLUDED_VARIABLE_ENCODER_H

#include "buffer.h"
#include "codec.h"

#include <iostream>
#include <optional>

template <MessageCodec T>
struct VariableLengthEncoder {
    using MessageType = T;

    static void write(T message, Buffer& buffer)
    {
        auto [data, length] = Codec<T>::as_bytes(message);
        buffer.append((const char*)&length, sizeof(uint32_t));
        std::cout
            << "[MESSAGE][RESPONDING][SENDING] Wrote length of the message: "
            << length << std::endl;
        buffer.append(data, length);
        std::cout << "[MESSAGE][RESPONDING][SENDING] Wrote message of length:"
                  << length << std::endl;
    }

    static std::optional<T> consume_message(Buffer& buffer)
    {
        if (buffer.size() < sizeof(uint32_t)) {
            return {};
        }
        uint32_t payload_size;

        buffer.cpy(&payload_size, sizeof(uint32_t));
        std::cout << "[MESSAGE][PARSING][CONSUME] payload_size="
                  << payload_size << ", buffer_size=" << buffer.size()
                  << std::endl;
        if (buffer.size() >= payload_size + sizeof(uint32_t)) {
            buffer.consume(sizeof(uint32_t));
            T message = Codec<T>::into(buffer, payload_size);
            buffer.consume(payload_size);
            return message;
        }
        return {};
    }
};

using StringEncoder = VariableLengthEncoder<std::string>;

#endif
