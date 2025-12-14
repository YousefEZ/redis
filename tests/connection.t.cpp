#include "connection.h"

#include <array>
#include <csignal>
#include <gtest/gtest.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

/**
class ConnectionTest {

  std::array<int, 2> m_fds;

  Connection m_server;
  Connection m_client;

  static std::array<int, 2> init_fds() {
    std::array<int, 2> fds;
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds.data()) == -1) {
      throw std::runtime_error("unable to init socketpair");
    }
    return fds;
  }

public:
  ConnectionTest()
      : m_fds{init_fds()}, m_server{m_fds[0]}, m_client{m_fds[1]} {}

  Connection &server_side() { return m_server; }

  Connection &client_side() { return m_client; }
};

TEST(ConnectionTest, TestSending) { EXPECT_TRUE(true); }

*/
