#ifndef INCLUDED_NET_SINGLE_TYPE_ENCODER_H
#define INCLUDED_NET_SINGLE_TYPE_ENCODER_H

#include "net_codec.h"

#include <optional>

namespace net {

template <template <typename> typename CODEC, typename T>
struct SingleTypeEncoder {
    using MessageType = T;

    template <WriteBuffer BUFFER>
    static void write(const T& message, BUFFER& buffer)
    {
        CODEC<T>::serialize(message, buffer);
    }

    template <ReadBuffer BUFFER>
    static std::optional<T> consume_message(BUFFER& buffer)
    {
        return CODEC<T>::deserialize(buffer);
    }
};

using StringEncoder = SingleTypeEncoder<Codec, std::string>;

}  // namespace net

#endif
