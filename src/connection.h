#ifndef INCLUDE_CONNECTION_H
#define INCLUDE_CONNECTION_H

#include <vector>

typedef int FileDescriptor;

struct Connection {
  FileDescriptor m_fd;

  bool m_want_read = false;
  bool m_want_write = false;
  bool m_want_close = false;

  std::vector<char> m_incoming;
  std::vector<char> m_outgoing;

  void read();
  void write();
};

#endif
