#include "connection.h"
#include "message_parsing.h"

#include <cerrno>
#include <iostream>

static const char ACK[] = "ACK";

void Connection::read() {
  std::cout << "[SERVER][CONNECTION][READ] receiving server message "
               "on connection fd: "
            << m_fd << std::endl;
  ssize_t rc = receive_message();
  if (rc < 0) {
    m_want_close = true;
    std::cout << "[SERVER][CONNECTION][READ] connection closed by peer fd: "
              << m_fd << std::endl;
    return;
  }
  // read the vector and determine whether the full message is there
  while (std::optional<std::string> message = consume_message(m_incoming)) {
    // do action on the read message
    std::cout << "[SERVER][CONNECTION][READ] message=" << message.value()
              << std::endl;
    m_want_write = true;
    m_want_read = false;
    m_outgoing.append(message.value().c_str(), message.value().size());
  }

  if (!m_outgoing.empty()) {
    m_want_write = true;
    m_want_read = false;
    write();
  }
}

void Connection::write() {
  if (m_outgoing.size() == 0) {
    m_want_read = true;
    m_want_write = false;
    return;
  }
  if (m_outgoing.write_to(m_fd, m_outgoing.size()) < 0 && errno != EAGAIN) {
    m_want_close = true;
    std::cout << "[SERVER][CONNECTION][WRITE] connection closed by peer fd: "
              << m_fd << std::endl;
    return;
  };
  m_outgoing.clear();
  std::cout << "[SERVER][CONNECTION][WRITE] sent server message on "
               "connection fd: "
            << m_fd << std::endl;
}

ssize_t Connection::receive_message() { return m_incoming.read_from(m_fd); }
