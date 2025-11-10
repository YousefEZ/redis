#ifndef MESSAGE_PARSING_H
#define MESSAGE_PARSING_H

#include <cstdint>
#include <unistd.h>

#define MAX_MESSAGE_LENGTH 4096

void send_message(int fd, const char *message, uint32_t length);

int receive_message(int fd, char buffer[MAX_MESSAGE_LENGTH]);

#endif
