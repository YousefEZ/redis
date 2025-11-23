#include "connection.h"
#include "message_parsing.h"

#include <iostream>

static const char ACK[] = "ACK";

void Connection::read() {
  std::cout << "[SERVER][CONNECTION][READ] receiving server message "
               "on connection fd: "
            << m_fd << std::endl;
  ssize_t rc = receive_message(m_fd, m_incoming);
  if (rc <= 0) {
    m_want_close = true;
    std::cout << "[SERVER][CONNECTION][READ] connection closed by peer fd: "
              << m_fd << std::endl;
    return;
  }
  // read the vector and determine whether the full message is there
  if (std::optional<std::string> message = consume_message(m_incoming)) {
    // do action on the read message
    std::cout << "[SERVER][CONNECTION][READ] message=" << message.value()
              << std::endl;
    m_want_write = true;
    m_want_read = false;
    m_outgoing.insert(m_outgoing.end(), ACK, ACK + sizeof(ACK));
  } else {
    std::cout << "[SERVER][CONNECTION][READ] incomplete message, waiting for "
                 "more data"
              << std::endl;
  }
}

void Connection::write() {
  if (m_outgoing.size() == 0) {
    m_want_read = true;
    m_want_write = false;
    return;
  }
  send_message(m_fd, m_outgoing.data(), m_outgoing.size());
  m_outgoing.clear();
  std::cout << "[SERVER][CONNECTION][WRITE] sent server message on "
               "connection fd: "
            << m_fd << std::endl;
}
