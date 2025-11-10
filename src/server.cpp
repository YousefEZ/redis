#include "server.h"
#include "message_parsing.h"
#include "utils.h"

#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Server::Server(sockaddr_in &&address) : m_address{address} {}

void Server::communicate(int fd) {
  char rbuf[MAX_MESSAGE_LENGTH] = {};
  receive_message(fd, rbuf);

  std::cout << "[SERVER][COMMUNICATE] received msg: " << rbuf << std::endl;

  char response[] = "ACK";
  send_message(fd, response, sizeof(response));
}

void Server::run() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  std::cout << "[SERVER][RUN] Created socket with fd: " << fd << std::endl;

  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  std::cout << "[SERVER][RUN] Binding to " << inet_ntoa(m_address.sin_addr)
            << ":" << ntohs(m_address.sin_port) << std::endl;

  int rc = bind(fd, (const sockaddr *)&m_address, sizeof(m_address));

  utils::die_on(rc, "[SERVER][RUN] unable to bind addr, shutting down");

  rc = listen(fd, SOMAXCONN);

  utils::die_on(
      rc, "[SERVER][RUN] unable to start listening on addr, shutting down");

  while (true) {
    struct sockaddr_in client_addr = {};

    socklen_t addrlen = sizeof(client_addr);

    int connfd = accept(fd, (sockaddr *)&client_addr, &addrlen);
    if (rc < 0) {
      continue; // there is an error
    }

    communicate(connfd);
  }
}
