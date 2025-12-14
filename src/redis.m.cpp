#include "client.h"
#include "connection.h"
#include "server.h"
#include "utils.h"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/ip.h>
#include <string>
#include <sys/socket.h>
#include <utility>

auto ip_address_value(const std::string &address) {
  in_addr addr;
  inet_pton(AF_INET, address.c_str(), &addr);
  return addr.s_addr;
}

void run_server() {

  std::string address, raw_port;

  std::cout << "[MAIN][SERVER] Enter the address to bind to: ";
  std::getline(std::cin, address);

  std::cout << "[MAIN][SERVER] Enter the port to bind to: ";
  std::getline(std::cin, raw_port);

  int port = std::stoi(raw_port);
  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ip_address_value(address);

  Server server{std::move(addr)};
  server.run();
}

Connection<StringEncoder> connect(sockaddr_in address) {

  FileDescriptor fd{socket(AF_INET, SOCK_STREAM, 0)};

  utils::die_on(fd < 0, "[CLIENT][RUN] unable to create socket, shutting down");

  int rc = connect(fd, (const sockaddr *)&address, sizeof(address));
  utils::die_on(rc, "[CLIENT][RUN] unable to connect to server, shutting down");
  return {std::move(fd)};
}

void run_client() {
  std::string address, raw_port;

  std::cout << "[MAIN][CLIENT] Enter the server address to connect to: ";
  std::getline(std::cin, address);

  std::cout << "[MAIN][CLIENT] Enter the server port to connect to: ";
  std::getline(std::cin, raw_port);

  int port = std::stoi(raw_port);
  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ip_address_value(address);

  Client client{connect(std::move(addr))};
  client.run();
}

int main() {
  std::cout << "[MAIN] Welcome to redis." << std::endl;

  std::cout << "[MAIN] To boot as a test client send 0" << std::endl;
  std::cout << "[MAIN] To boot as a server send 1" << std::endl;

  std::string input;
  std::getline(std::cin, input);

  int choice = std::stoi(input);

  switch (std::stoi(input)) {
  case 0:
    run_client();
    break;
  case 1:
    run_server();
    break;
  }

  return 0;
}
