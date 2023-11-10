#ifndef SOCKS5_PROXY_MAIN_SERVER_HPP
#define SOCKS5_PROXY_MAIN_SERVER_HPP

// ReSharper disable once CppUnusedIncludeDirective
#include <iostream>
#include <map>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>

namespace SOCKS5 {
    [[maybe_unused]] constexpr unsigned char VERSION = 0x05;
    [[maybe_unused]] constexpr unsigned char NO_AUTH = 0x00;
    [[maybe_unused]] constexpr unsigned char NO_ACCEPTABLE_METHODS = 0xff;
    [[maybe_unused]] constexpr unsigned char CONNECT = 0x01;
    [[maybe_unused]] constexpr unsigned char BIND = 0x02;
    [[maybe_unused]] constexpr unsigned char UDP_ASSOCIATE = 0x03;
    [[maybe_unused]] constexpr unsigned char RSV = 0x0;
    [[maybe_unused]] constexpr unsigned char IP_V4 = 0x01;
    [[maybe_unused]] constexpr unsigned char DOMAIN_NAME = 0x03;
    [[maybe_unused]] constexpr unsigned char IP_V6 = 0x04;
    [[maybe_unused]] constexpr unsigned char SUCCESS = 0x00;
    [[maybe_unused]] constexpr unsigned char SERVER_FAILURE = 0x01;
    [[maybe_unused]] constexpr unsigned char CONNECTION_NOT_ALLOWED = 0x02;
    [[maybe_unused]] constexpr unsigned char NETWORK_UNREACHABLE = 0x03;
    [[maybe_unused]] constexpr unsigned char HOST_UNREACHABLE = 0x04;
    [[maybe_unused]] constexpr unsigned char CONNECTION_REFUSED = 0x05;
    [[maybe_unused]] constexpr unsigned char TTL_EXPIRED = 0x06;
    [[maybe_unused]] constexpr unsigned char COMMAND_NOT_SUPPORTED = 0x07;
    [[maybe_unused]] constexpr unsigned char ADDRESS_TYPE_NOT_SUPPORTED = 0x08;


    namespace asio = boost::asio;
    using tcp = asio::ip::tcp;

    class Main_server {
        unsigned short port;

        std::map<std::string, asio::ip::address_v4> resolved_hosts;

        asio::awaitable<void> start();

        asio::awaitable<void> client_handler(tcp::socket client_socket);

        static asio::awaitable<bool> validate_welcome_message(const std::array<unsigned char, 3>&welcome_message);

        static unsigned char select_auth_method(const std::array<unsigned char, 3>&welcome_message);

        static asio::awaitable<void> send_auth_response(tcp::socket&socket, const unsigned char&method);

        static asio::awaitable<std::vector<unsigned char>> get_client_request(tcp::socket&socket);

        asio::awaitable<tcp::endpoint> handle_client_request(std::vector<unsigned char>&request);

        static unsigned char get_connect_error(const unsigned char&cmd, const boost::system::error_code&code);


        // what is [[nodiscard]]
        asio::awaitable<void> handle_client_data(tcp::socket client_socket, tcp::socket remote_socket) const;

        static asio::awaitable<void>
        remote_to_client(const std::shared_ptr<tcp::socket>& remote_socket, const std::shared_ptr<tcp::socket>& client_socket);

        static asio::awaitable<void>
        client_to_remote(const std::shared_ptr<tcp::socket>& client_socket, const std::shared_ptr<tcp::socket>& remote_socket);

    public:
        explicit Main_server(unsigned short port_);

        void start_server();
    };
}


#endif //SOCKS5_PROXY_MAIN_SERVER_HPP
