#include "client.h"
#include "buffer.h"
#include "message_parsing.h"
#include "utils.h"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <unistd.h>

Client::Client(sockaddr_in address) : addr(address) {}

void Client::run() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  utils::die_on(fd < 0, "[CLIENT][RUN] unable to create socket, shutting down");

  int rc = connect(fd, (const sockaddr *)&addr, sizeof(addr));
  utils::die_on(rc, "[CLIENT][RUN] unable to connect to server, shutting down");

  while (true) {
    std::cout << "Enter the message to send to the server:";

    std::string message;
    std::getline(std::cin, message);
    rc = send_message(fd, message.c_str(), message.size());
    if (rc < 0) {
      std::cerr << "[CLIENT][RUN] send_message() error, shutting down"
                << std::endl;
      break;
    }
    Buffer rbuf(1024);

    rc = rbuf.read_from(fd);

    utils::die_on(rc < 0, "[CLIENT][RUN] read() error");

    std::optional<std::string> msg = consume_message(rbuf);
    if (msg.has_value())
      std::cout << "[CLIENT][RUN] received response: " << msg.value()
                << std::endl;
    else {
      std::cerr << "[CLIENT][RUN] malformed response received" << std::endl;
      break;
    }
  }
  close(fd);
}
