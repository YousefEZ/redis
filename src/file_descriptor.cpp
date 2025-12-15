#include "file_descriptor.h"

#include <utility>

FileDescriptor::FileDescriptor(int fd) : m_fd{fd} {}

FileDescriptor::FileDescriptor(FileDescriptor &&other) : m_fd{other.m_fd} {
  other.m_fd = -1;
}

FileDescriptor &FileDescriptor::operator=(FileDescriptor &&other) {
  std::swap(this->m_fd, other.m_fd);
  return *this;
}

FileDescriptor::~FileDescriptor() {
  if (m_fd >= 0) {
    close(m_fd);
  }
}

void FileDescriptor::as_non_blocking() const {
  fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL, 0) | O_NONBLOCK);
}

FileDescriptor::operator int() const { return m_fd; }
