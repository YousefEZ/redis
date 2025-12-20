#ifndef REDIS_SERVER_H
#define REDIS_SERVER_H

#include "connection.h"
#include "variable_encoder.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <vector>

typedef Connection<StringEncoder> StringConnection;

class ServerProcessor {
  public:
    std::optional<std::string> process(const std::string& message);
};

class Server {
    const sockaddr_in             m_address;
    std::vector<StringConnection> m_connections;
    ServerProcessor               m_processor;

    [[nodiscard]] const StringConnection& socket_connection() const;

    void execute_connection_events(const pollfd&     poll,
                                   StringConnection& connection);
    void remove_closed_connections();
    void accept_connection(const pollfd& socket_poll);
    void check_connections(const std::vector<pollfd>& poll_event);

  public:
    void run();

    Server(sockaddr_in&& address);
};

#endif
