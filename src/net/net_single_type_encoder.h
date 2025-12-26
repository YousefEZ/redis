#ifndef INCLUDED_SINGLE_TYPE_ENCODER_H
#define INCLUDED_SINGLE_TYPE_ENCODER_H

#include "net_buffer.h"
#include "net_codec.h"

#include <optional>

template <template <typename> typename CODEC, typename T>
struct SingleTypeEncoder {
    using MessageType = T;

    static void write(const T& message, Buffer& buffer)
    {
        CODEC<T>::serialize(message, buffer);
    }

    static std::optional<T> consume_message(Buffer& buffer)
    {
        return CODEC<T>::deserialize(buffer);
    }
};

using StringEncoder = SingleTypeEncoder<Codec, std::string>;

#endif
