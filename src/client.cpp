#include "client.h"
#include "utils.h"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>

Client::Client(sockaddr_in address) : addr(address) {}

void Client::run() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  utils::die_on(fd < 0, "[CLIENT][RUN] unable to create socket, shutting down");

  int rc = connect(fd, (const sockaddr *)&addr, sizeof(addr));
  utils::die_on(rc, "[CLIENT][RUN] unable to connect to server, shutting down");

  const char message[] = "Hello, Server!";
  ssize_t n = write(fd, message, sizeof(message));

  char rbuf[64] = {};
  n = read(fd, rbuf, sizeof(rbuf) - 1);
  utils::die_on(n < 0, "[CLIENT][RUN] read() error");

  std::cout << "[CLIENT][RUN] received response: " << rbuf << std::endl;

  close(fd);
}
