#ifndef INCLUDED_REDIS_ENCODER_H
#define INCLUDED_REDIS_ENCODER_H

#include <net_codec.h>
#include <net_tagged_encoder.h>

using MessageTypes = net::Messages<std::string, uint32_t>;
using KeyEncoder   = net::TaggedEncoder<net::Codec, MessageTypes>;

#endif
