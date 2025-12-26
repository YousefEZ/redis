#ifndef INCLUDED_NET_ENCODER_H
#define INCLUDED_NET_ENCODER_H

#include "net_buffer.h"

#include <concepts>
#include <optional>

namespace net {

template <typename ENCODER>
concept Encoder = requires(ENCODER                     encoder,
                           Buffer&                     buffer,
                           const ENCODER::MessageType& message)
{
    {ENCODER::write(message, buffer)}->std::same_as<void>;
    {ENCODER::consume_message(buffer)}
        ->std::same_as<std::optional<typename ENCODER::MessageType> >;
};
}  // namespace net

#endif
