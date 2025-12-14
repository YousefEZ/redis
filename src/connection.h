#ifndef INCLUDE_CONNECTION_H
#define INCLUDE_CONNECTION_H

#include "buffer.h"
#include "message_parsing.h"

#include <fcntl.h>
#include <iostream>
#include <optional>
#include <sys/poll.h>
#include <sys/types.h>
#include <utility>

#define MAX_BUFFER_SIZE 4096

struct FileDescriptor {
  int m_fd{-1};

  FileDescriptor(int fd) : m_fd{fd} {}
  FileDescriptor(const FileDescriptor &) = delete;
  FileDescriptor(FileDescriptor &&other) : m_fd{other.m_fd} { other.m_fd = -1; }
  FileDescriptor &operator=(const FileDescriptor &) = delete;
  FileDescriptor &operator=(FileDescriptor &&other) {
    std::swap(this->m_fd, other.m_fd);
    return *this;
  }

  ~FileDescriptor() {
    if (m_fd >= 0) {
      close(m_fd);
    }
  }

  void as_non_blocking() const {
    fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL, 0) | O_NONBLOCK);
  }
  operator int() const { return m_fd; }
};

template <typename PROCESSOR, typename T>
concept Processor = requires(PROCESSOR &processor, T message) {
  { processor.process(message) } -> std::same_as<std::optional<T>>;
};

enum struct Signals : uint8_t {
  e_NONE = 0x0,
  e_READ = 0x1,
  e_WRITE = 0x2,
  e_CLOSE = 0x4
};

inline Signals operator&(Signals a, Signals b) {
  return static_cast<Signals>(static_cast<uint8_t>(a) &
                              static_cast<uint8_t>(b));
}

inline Signals operator|(Signals a, Signals b) {
  return static_cast<Signals>(static_cast<uint8_t>(a) |
                              static_cast<uint8_t>(b));
}

inline Signals &operator&=(Signals &a, Signals b) {
  a = a & b;
  return a;
}

inline Signals operator~(Signals a) {
  return static_cast<Signals>(~static_cast<uint8_t>(a));
}

inline Signals &operator|=(Signals &a, Signals b) {
  a = a | b;
  return a;
}

template <Encoder ENCODER, ssize_t BUF_SIZE = MAX_BUFFER_SIZE>
class Connection {

  typedef ENCODER::MessageType MessageType;
  FileDescriptor m_fd;

  Signals m_signals;

  Buffer m_incoming{BUF_SIZE};
  Buffer m_outgoing{BUF_SIZE};

  void read();

public:
  Connection(FileDescriptor &&fd, Signals signals = Signals::e_NONE)
      : m_fd{std::move(fd)}, m_signals(signals) {}
  const FileDescriptor &fd() const { return m_fd; }

  bool want_read() const { return (bool)(m_signals & Signals::e_READ); }

  bool want_write() const { return (bool)(m_signals & Signals::e_WRITE); }

  bool want_close() const { return (bool)(m_signals & Signals::e_CLOSE); }

  void send(MessageType &&message, uint32_t length);
  void send(const MessageType &message, uint32_t length);

  void write();

  template <typename PROCESSOR>
    requires Processor<PROCESSOR, MessageType>
  void process(PROCESSOR &processor);

  void close() { m_signals |= Signals::e_CLOSE; }
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
    m_signals |= Signals::e_READ;
    m_signals &= ~Signals::e_WRITE;
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
  m_signals |= Signals::e_WRITE;
  m_signals &= ~Signals::e_READ;
  write();
}

template <Encoder ENCODER, ssize_t BUF_SIZE>
void Connection<ENCODER, BUF_SIZE>::send(const MessageType &message,
                                         uint32_t length) {
  ENCODER::write(message, m_outgoing);
  m_signals |= Signals::e_WRITE;
  m_signals &= ~Signals::e_READ;
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
      m_signals |= Signals::e_WRITE;
      m_signals &= ~Signals::e_READ;
    }
  }

  if (!m_outgoing.empty()) {
    // try writing right away if its full then EAGAIN will be handled
    write();
  }
}

#endif
