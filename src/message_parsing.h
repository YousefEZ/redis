#ifndef MESSAGE_PARSING_H
#define MESSAGE_PARSING_H

#include "buffer.h"
#include <cstdint>
#include <optional>
#include <string>
#include <unistd.h>
#include <vector>

void write_message(const char *message, uint32_t length, Buffer &buffer);

std::optional<std::string> consume_message(Buffer &buffer);

ssize_t receive_message(int fd, std::vector<char> &buffer);

#endif
