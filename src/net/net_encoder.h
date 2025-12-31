#ifndef INCLUDED_NET_ENCODER_H
#define INCLUDED_NET_ENCODER_H

#include <concepts>
#include <optional>

namespace net {

template <typename ENCODER, typename BUFFER>
concept Encoder = requires(BUFFER & buffer,
                           const ENCODER::MessageType& message)
{
    {ENCODER::write(message, buffer)}->std::same_as<void>;
};

template <typename ENCODER, typename BUFFER>
concept Decoder = requires(BUFFER & buffer)
{
    {ENCODER::consume_message(buffer)}
        ->std::same_as<std::optional<typename ENCODER::MessageType> >;
};

template <typename ENCODER, typename WRITE_BUFFER, typename READ_BUFFER>
concept Serde = Encoder<ENCODER, WRITE_BUFFER> &&
                Decoder<ENCODER, READ_BUFFER>;

}  // namespace net

#endif
