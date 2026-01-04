#include <redis_client.h>
#include <redis_server.h>

#include <net_server.h>

#include <arpa/inet.h>
#include <iostream>
#include <netinet/ip.h>
#include <string>
#include <sys/socket.h>

auto ip_address_value(const std::string& address)
{
    in_addr addr;
    inet_pton(AF_INET, address.c_str(), &addr);
    return addr.s_addr;
}

void run_server()
{
    std::string address, raw_port;

    std::cout << "[MAIN][SERVER] Enter the address to bind to: ";
    std::getline(std::cin, address);

    std::cout << "[MAIN][SERVER] Enter the port to bind to: ";
    std::getline(std::cin, raw_port);

    int         port     = std::stoi(raw_port);
    sockaddr_in addr     = {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = ip_address_value(address);

    redis::RedisProcessor processor;
    redis::RedisServer    server{std::move(addr), processor};
    server.run();
}

int main()
{
    std::cout << "[MAIN] Welcome to redis." << std::endl;
    std::cout << "[MAIN] Booting a server" << std::endl;
    run_server();

    return 0;
}
