#ifndef INCLUDED_REDIS_ENCODER_H
#define INCLUDED_REDIS_ENCODER_H

#include "net_single_type_encoder.h"
#include <net_codec.h>
#include <net_tagged_encoder.h>

// using MessageTypes = net::Messages<std::string, uint32_t>;
//  using KeyEncoder   = net::TaggedEncoder<net::Codec, MessageTypes>;
using KeyEncoder = net::SingleTypeEncoder<net::Codec, std::string>;

#endif
