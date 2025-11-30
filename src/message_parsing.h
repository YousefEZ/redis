#ifndef MESSAGE_PARSING_H
#define MESSAGE_PARSING_H

#include "buffer.h"
#include <cstdint>
#include <optional>
#include <string>
#include <unistd.h>
#include <vector>

ssize_t send_message(int fd, const char *message, uint32_t length);

std::optional<std::string> consume_message(Buffer &buffer);

ssize_t receive_message(int fd, std::vector<char> &buffer);

#endif
