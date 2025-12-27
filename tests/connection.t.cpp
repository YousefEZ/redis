#include "net_codec.h"
#include <net_connection.h>
#include <net_server.h>
#include <net_single_type_encoder.h>

#include <array>
#include <csignal>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class StringConnectionTest : public testing::Test {
    std::array<int, 2> m_fds;
    typedef net::Connection<net::SingleTypeEncoder<net::Codec, std::string> >
        StringConnection;

    StringConnection m_server;
    StringConnection m_client;

    static std::array<int, 2> init_fds()
    {
        std::array<int, 2> fds;
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds.data()) == -1) {
            throw std::runtime_error("unable to init socketpair");
        }
        return fds;
    }

  public:
    StringConnectionTest()
    : m_fds{init_fds()}
    , m_server{m_fds[0]}
    , m_client{m_fds[1]}
    {
    }

    StringConnection& server_side() { return m_server; }
    StringConnection& client_side() { return m_client; }
};

struct StringProcessor {
    std::optional<std::string> m_held_message;

    std::optional<std::string> process(const std::string& message)
    {
        m_held_message = message;
        return message;
    }
};

TEST_F(StringConnectionTest, TestSendAndReceive)
{
    std::string message = "Hello, World!";
    server_side().send(message);

    StringProcessor processor;
    client_side().process(processor);
    EXPECT_TRUE(processor.m_held_message.has_value() &&
                *processor.m_held_message == message);
}
