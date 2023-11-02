#include "Data_handler.hpp"


namespace SOCKS5 {
    Data_handler::Data_handler(unsigned short port_, boost::asio::ip::tcp::socket client_socket_,
                                       boost::asio::ip::tcp::socket remote_socket_) :
            port(port_),
            client_socket(std::move(client_socket_)),
            remote_socket(std::move(remote_socket_)) {}

    asio::awaitable<void> Data_handler::handle_client_data() {
        std::cout << Colors::YELLOW << "\tSTART OF RECEIVING DATA" << Colors::RESET << std::endl;

        co_return;
    }
}