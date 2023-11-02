#ifndef SOCKS5_PROXY_DATA_HANDLER_HPP
#define SOCKS5_PROXY_DATA_HANDLER_HPP


#include <boost/asio.hpp>
#include <iostream>
#include "Colors.hpp"

namespace SOCKS5 {

    namespace asio = boost::asio;
    using tcp = asio::ip::tcp;


    class Data_handler {
    private:
        tcp::socket client_socket;
        tcp::socket remote_socket;
        unsigned short port;
    public:
        Data_handler(unsigned short port_, tcp::socket client_socket_, tcp::socket remote_socket_);


        asio::awaitable<void> handle_client_data();
    };
}


#endif //SOCKS5_PROXY_DATA_HANDLER_HPP
