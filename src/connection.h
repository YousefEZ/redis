#ifndef INCLUDE_CONNECTION_H
#define INCLUDE_CONNECTION_H

#include "buffer.h"
#include "encoder.h"
#include "file_descriptor.h"
#include "signals.h"

#include <iostream>
#include <optional>
#include <sys/poll.h>
#include <sys/types.h>
#include <utility>

#define MAX_BUFFER_SIZE 4096

template <typename PROCESSOR, typename T>
concept Processor = requires(PROCESSOR &processor, T message) {
  { processor.process(message) } -> std::same_as<std::optional<T>>;
};

struct Signals {
  bool read;
  bool write;
  bool close;
};

template <Encoder ENCODER, ssize_t BUF_SIZE = MAX_BUFFER_SIZE>
class Connection {

  typedef ENCODER::MessageType MessageType;
  FileDescriptor m_fd;

  Signals m_signals;

  Buffer m_incoming{BUF_SIZE};
  Buffer m_outgoing{BUF_SIZE};

  void read();

public:
  Connection(FileDescriptor &&fd, Signals signals = {})
      : m_fd{std::move(fd)}, m_signals(signals) {}
  const FileDescriptor &fd() const { return m_fd; }

  bool want_read() const { return (bool)(m_signals.read); }

  bool want_write() const { return (bool)(m_signals.write); }

  bool want_close() const { return (bool)(m_signals.close); }

  void send(MessageType &&message, uint32_t length);
  void send(const MessageType &message, uint32_t length);

  void write();

  template <typename PROCESSOR>
    requires Processor<PROCESSOR, MessageType>
  void process(PROCESSOR &processor);

  void close() { m_signals.close = true; }
};

template <Encoder ENCODER, ssize_t BUF_SIZE>
void Connection<ENCODER, BUF_SIZE>::read() {
  std::cout << "[SERVER][CONNECTION][READ] receiving server message "
               "on connection fd: "
            << m_fd << std::endl;
  ssize_t rc = m_incoming.read_from(m_fd);
  if (rc < 0) {
    close();
    std::cout << "[SERVER][CONNECTION][READ] connection closed by peer fd: "
              << m_fd << std::endl;
    return;
  }
}

template <Encoder ENCODER, ssize_t BUF_SIZE>
void Connection<ENCODER, BUF_SIZE>::write() {
  if (m_outgoing.size() == 0) {
    m_signals.read = true;
    m_signals.write = false;
    return;
  }
  if (m_outgoing.write_to(m_fd, m_outgoing.size()) < 0) {
    if (errno != EAGAIN) {
      close();
      std::cout << "[SERVER][CONNECTION][WRITE] connection closed by peer fd: "
                << m_fd << std::endl;
      return;
    }
    std::cout << "[SERVER][CONNECTION][WRITE] EAGAIN on connection fd: " << m_fd
              << std::endl;
  } else {
    m_outgoing.clear();
    std::cout << "[SERVER][CONNECTION][WRITE] sent server message on "
                 "connection fd: "
              << m_fd << std::endl;
  }
}

template <Encoder ENCODER, ssize_t BUF_SIZE>
void Connection<ENCODER, BUF_SIZE>::send(MessageType &&message,
                                         uint32_t length) {
  ENCODER::write(std::move(message), m_outgoing);
  m_signals.write = true;
  m_signals.read = false;
  write();
}

template <Encoder ENCODER, ssize_t BUF_SIZE>
void Connection<ENCODER, BUF_SIZE>::send(const MessageType &message,
                                         uint32_t length) {
  ENCODER::write(message, m_outgoing);
  m_signals.write = true;
  m_signals.read = false;
  write();
}

template <Encoder ENCODER, ssize_t BUF_SIZE>
template <typename PROCESSOR>
  requires Processor<PROCESSOR, typename ENCODER::MessageType>
void Connection<ENCODER, BUF_SIZE>::process(PROCESSOR &processor) {
  read();
  while (std::optional<MessageType> message =
             ENCODER::consume_message(m_incoming)) {
    // do action on the read message
    std::cout << "[SERVER][CONNECTION][READ] message=" << message.value()
              << std::endl;
    if (std::optional<MessageType> response =
            processor.process(std::move(*message))) {
      ENCODER::write(std::move(*response), m_outgoing);
      m_signals.write = true;
      m_signals.read = false;
    }
  }

  if (!m_outgoing.empty()) {
    // try writing right away if its full then EAGAIN will be handled
    write();
  }
}

#endif
