#ifndef SOCKS5_PROXY_MAIN_SERVER_HPP
#define SOCKS5_PROXY_MAIN_SERVER_HPP

#include <boost/asio/awaitable.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <iostream>

#include "Colors.hpp"

namespace SOCKS5 {
    const unsigned char VERSION = 0x05;
    const unsigned char NO_AUTH = 0x00;
    const unsigned char NO_ACCEPTABLE_METHODS = 0xff;
    [[maybe_unused]] const unsigned char CONNECT = 0x01;
    [[maybe_unused]] const unsigned char BIND = 0x02;
    [[maybe_unused]] const unsigned char UDP_ASSOCIATE = 0x03;
    [[maybe_unused]] const unsigned char RSV = 0x0;
    [[maybe_unused]] const unsigned char IP_V4 = 0x01;
    [[maybe_unused]] const unsigned char DOMAINNAME = 0x03;
    [[maybe_unused]] const unsigned char IP_V6 = 0x04;
    [[maybe_unused]] const unsigned char SUCCESS = 0x00;

    namespace asio = boost::asio;
    using tcp = asio::ip::tcp;

    class Main_server {

    private:
        unsigned short port;
        boost::asio::awaitable<void> start();
        asio::awaitable<void> client_handler(tcp::socket client_socket);
        asio::awaitable<bool> validate_welcome_message(const std::array<unsigned char, 3>& welcome_message);
        unsigned char select_auth_method(const std::array<unsigned char, 3>& welcome_message);
        asio::awaitable<void> send_auth_response(tcp::socket& socket, const unsigned char& method);
        asio::awaitable <std::vector<unsigned char>> get_client_request(tcp::socket& socket);
        asio::awaitable<tcp::endpoint> handle_client_request(std::vector<unsigned char>& request);
        unsigned char get_connect_error(const unsigned char &cmd, const boost::system::error_code &code);
        asio::awaitable<void> handle_client_data(tcp::socket client_socket, tcp::socket remote_socket);
        asio::awaitable<void> remote_to_client(std::shared_ptr<tcp::socket> remote_socket, std::shared_ptr<tcp::socket> client_socket);
        asio::awaitable<void> client_to_remote(std::shared_ptr<tcp::socket> client_socket, std::shared_ptr<tcp::socket> remote_socket);

    public:

        explicit Main_server(unsigned short port_);

        void start_server();


    };
}


#endif //SOCKS5_PROXY_MAIN_SERVER_HPP
