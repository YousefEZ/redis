#ifndef INCLUDED_VARIABLE_ENCODER_H
#define INCLUDED_VARIABLE_ENCODER_H

#include "buffer.h"
#include "codec.h"

#include <optional>

template <MessageCodec T>
struct VariableLengthEncoder {
    using MessageType = T;

    static void write(T message, Buffer& buffer)
    {
        Codec<T>::serialize(message, buffer);
    }

    static std::optional<T> consume_message(Buffer& buffer)
    {
        return Codec<T>::deserialize(buffer);
    }
};

using StringEncoder = VariableLengthEncoder<std::string>;

#endif
