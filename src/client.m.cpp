#include <redis_client.h>
#include <redis_server.h>

#include <net_connection.h>
#include <net_server.h>
#include <net_utils.h>

#include <arpa/inet.h>
#include <iostream>
#include <netinet/ip.h>
#include <string>
#include <sys/socket.h>
#include <utility>

auto ip_address_value(const std::string& address)
{
    in_addr addr;
    inet_pton(AF_INET, address.c_str(), &addr);
    return addr.s_addr;
}

redis::SyncConnection connect(sockaddr_in address)
{
    net::FileDescriptor fd{socket(AF_INET, SOCK_STREAM, 0)};

    net::utils::die_on(fd < 0,
                       "[CLIENT][RUN] unable to create socket, shutting down");

    int rc = connect(fd, (const sockaddr*)&address, sizeof(address));
    net::utils::die_on(
        rc,
        "[CLIENT][RUN] unable to connect to server, shutting down");
    return {std::move(fd)};
}

void run_client()
{
    std::string address, raw_port;

    std::cout << "[MAIN][CLIENT] Enter the server address to connect to: ";
    std::getline(std::cin, address);

    std::cout << "[MAIN][CLIENT] Enter the server port to connect to: ";
    std::getline(std::cin, raw_port);

    int         port     = std::stoi(raw_port);
    sockaddr_in addr     = {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = ip_address_value(address);
    redis::SyncClient client{connect(std::move(addr))};
    while (true) {
        std::cout << "Enter the message to send to the server:";

        std::string message;
        std::getline(std::cin, message);

        auto response = client.request(std::move(message));
        std::cout << "[MAIN][CLIENT] received response: " << response
                  << std::endl;
    }
}

int main()
{
    std::cout << "[MAIN] Welcome to redis." << std::endl;

    std::cout << "[MAIN] booting a test client" << std::endl;
    run_client();

    return 0;
}
