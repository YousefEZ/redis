#ifndef INCLUDE_CONNECTION_H
#define INCLUDE_CONNECTION_H

#include "buffer.h"

#define MAX_BUFFER_SIZE 4096

typedef int FileDescriptor;

struct Connection {
  FileDescriptor m_fd;

  bool m_want_read = false;
  bool m_want_write = false;
  bool m_want_close = false;

  Buffer m_incoming{MAX_BUFFER_SIZE};
  Buffer m_outgoing{MAX_BUFFER_SIZE};

  void read();
  void write();
  ssize_t receive_message();
};

#endif
