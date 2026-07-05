#include <redis_client.h>
#include <redis_schema.h>
#include <redis_server.h>

#include <net_connection.h>
#include <net_server.h>
#include <net_utils.h>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/socket.h>
#include <type_traits>
#include <utility>
#include <variant>

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

    SPDLOG_INFO("[MAIN][CLIENT] Enter the server address to connect to: ");
    std::getline(std::cin, address);

    SPDLOG_INFO("[MAIN][CLIENT] Enter the server port to connect to: ");
    std::getline(std::cin, raw_port);

    int         port     = std::stoi(raw_port);
    sockaddr_in addr     = {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = ip_address_value(address);
    redis::SyncClient client{connect(std::move(addr))};
    while (true) {
        SPDLOG_INFO("Enter the message to send to the server:");

        std::string message;
        std::string command;
        std::cin >> command;

        SPDLOG_DEBUG("[MAIN][CLIENT] parsing command: '{}'", command);
        if (command == "get") {
            std::string key;
            std::cin >> key;
            SPDLOG_DEBUG("[MAIN][CLIENT] GET KEY: '{}'", key);
            auto response = client.get(key);
            std::visit(
                [](auto&& arg) {
                    SPDLOG_DEBUG("[MAIN][CLIENT] ARG TYPE: {}",
                                 typeid(arg).name());
                    if constexpr (std::is_same_v<
                                      std::decay_t<decltype(arg)>,
                                      redis::GetResponse<std::string> >) {
                        SPDLOG_INFO("[MAIN][CLIENT] received GET response: {}",
                                    arg.value);
                    }
                },
                response);
        }
        else if (command == "set") {
            std::string key;
            std::cin >> key;
            SPDLOG_DEBUG("[MAIN][CLIENT] SET KEY: '{}'", key);
            std::string value;
            std::cin >> value;
            SPDLOG_DEBUG("[MAIN][CLIENT] TO VALUE: '{}'", value);
            auto response = client.set(key, value);
            std::visit(
                [](auto&& arg) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>,
                                                 redis::SetResponse>) {
                        SPDLOG_INFO("[MAIN][CLIENT] received SET response: {}",
                                    arg.success);
                    }
                },
                response);
        }
    }
}

int main()
{
    SPDLOG_INFO("[MAIN] Welcome to redis.");

    SPDLOG_INFO("[MAIN] booting a test client");
    run_client();

    return 0;
}
