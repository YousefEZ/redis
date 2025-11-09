#include "client.h"
#include "server.h"

#include <iostream>
#include <string>

int main() {
  std::cout << "Welcome to redis." << std::endl;

  std::cout << "To boot as a test client send 0" << std::endl;
  std::cout << "To boot as a server send 1" << std::endl;

  std::string input;
  std::getline(std::cin, input);

  int choice = std::stoi(input);

  switch (std::stoi(input)) {
  case 0:
    Client client;
    client.run();
    break;
  case 1:
    Server server;
    server.run();
    break;
  }

  return 0;
}
