#include "client.h"
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
    send_message(fd, message.c_str(), message.size());

    char rbuf[MAX_MESSAGE_LENGTH] = {};

    rc = receive_message(fd, rbuf);
    utils::die_on(rc, "[CLIENT][RUN] read() error");

    std::cout << "[CLIENT][RUN] received response: " << rbuf << std::endl;
  }
  close(fd);
}
