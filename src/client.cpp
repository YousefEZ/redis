#include "client.h"
#include "connection.h"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <unistd.h>

namespace {

class ClientProcessor {
  public:
    std::optional<std::string> process(std::string message)
    {
        std::cout << "[CLIENT][PROCESSOR] processing message: " << message
                  << std::endl;
        return "ACK";
    }
};

}  // namespace

void Client::run()
{
    ClientProcessor processor;
    while (true) {
        std::cout << "Enter the message to send to the server:";

        std::string message;
        std::getline(std::cin, message);

        m_conn.send(std::move(message));

        m_conn.process(processor);
    }
}
