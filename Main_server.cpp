#include "Main_server.hpp"


namespace SOCKS5 {

    namespace asio = boost::asio;
    using tcp = asio::ip::tcp;

    Main_server::Main_server(unsigned short port_) : port(port_) {}

    void Main_server::start_server() {
        asio::io_context ioContext;
        asio::co_spawn(ioContext, [this] { return start(); }, asio::detached);
        ioContext.run();
    }

    asio::awaitable<void> Main_server::start() {
        const auto executor = co_await asio::this_coro::executor; // обязательно надо дождаться executor, иначе как запускать корутины?

        tcp::acceptor acceptor{executor, {tcp::v4(), this->port}};

        for (;;) {
            tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
            asio::co_spawn(executor, [this, &socket] { return client_handler(std::move(socket)); }, asio::detached);
        }

    }

    asio::awaitable<void> Main_server::client_handler(tcp::socket client_socket) {
        std::cout << Colors::GREEN << "New connection: " << client_socket.remote_endpoint().address() << ':'
                  << client_socket.remote_endpoint().port() << Colors::RESET << std::endl;
        std::array<unsigned char, 3> welcome_message{};
        co_await asio::async_read(client_socket, boost::asio::buffer(welcome_message),
                                  asio::use_awaitable); // use_awaitable нужен, чтобы дождаться
        bool is_valid = co_await this->validate_welcome_message(welcome_message);
        if (!is_valid) {
            client_socket.close();
            co_return;
        }
        co_await this->send_auth_response(client_socket, select_auth_method(welcome_message));

        auto client_req = co_await this->get_client_request(client_socket);
        auto endpoint = co_await this->handle_client_request(client_req);
        std::cout << Colors::CYAN << "\tRequest - host: " << endpoint.address() << ':' << endpoint.port() << Colors::RESET << std::endl;
        co_return;
    }

    asio::awaitable<bool> Main_server::validate_welcome_message(const std::array<unsigned char, 3> &welcome_message) {
        std::cout << Colors::BLUE << '\t' << "VER=" << static_cast<int>(welcome_message[0]) << ' ';
        std::cout << "NMETHODS=" << static_cast<int>(welcome_message[1]) << ' ';
        std::cout << "METHOD=" << static_cast<int>(welcome_message[2]) << Colors::RESET << std::endl;
        if (!(welcome_message[0] == SOCKS5::VERSION && welcome_message[2] == SOCKS5::NO_AUTH)) {
            co_return false;
        }
        co_return true;
    }


    unsigned char Main_server::select_auth_method(
            const std::array<unsigned char, 3> &welcome_message) {
        unsigned char nmethods = welcome_message[1];
        for (unsigned char i = 2; i < nmethods + 2; ++i) {
            if (welcome_message[i] == SOCKS5::NO_AUTH) {
                return welcome_message[i];
            }
        }
        return SOCKS5::NO_ACCEPTABLE_METHODS;
    }


    asio::awaitable<void> Main_server::send_auth_response(tcp::socket &socket, const unsigned char &method) {
        std::array<unsigned char, 2> response{SOCKS5::VERSION, method};
        co_await asio::async_write(socket, asio::buffer(response), asio::use_awaitable);
        std::cout << Colors::MAGENTA << "\tSent auth method to client: " << static_cast<int>(method) << Colors::RESET
                  << std::endl;
        co_return;
    }

    asio::awaitable<std::vector<unsigned char>> Main_server::get_client_request(tcp::socket &socket) {
        asio::streambuf buffer;
        std::size_t n = co_await asio::async_read(socket, buffer, asio::transfer_at_least(1), asio::use_awaitable);
        std::vector<unsigned char> request(asio::buffers_begin(buffer.data()), asio::buffers_end(buffer.data()));
        //std::cout << n << std::endl;
        co_return request;

    }

    asio::awaitable<tcp::endpoint> Main_server::handle_client_request(std::vector<unsigned char> &request) {
        auto cmd = request[1];
        if (cmd != SOCKS5::CONNECT) {
            std::cerr << "Unsupported operation: " << static_cast<int>(cmd) << std::endl;
            co_return tcp::endpoint{tcp::v6(), 0};
        }
        std::cout << Colors::YELLOW << "\tRequest - version: " << static_cast<int>(request[0]) <<
                  ", CMD: " << static_cast<int>(request[1]) <<
                  ", ATYP: " << static_cast<int>(request[3]) << Colors::RESET << std::endl;


        unsigned short dest_port;
        std::array<unsigned char, 4> ip{};
        std::string ipStr;
        switch (request[3]) {
            case SOCKS5::IP_V4:
                for (int i = 0; i < 4; ++i) {
                    ip[i] = request[i + 4];
                }
                ipStr =
                        std::to_string(ip[0]) + "." + std::to_string(ip[1]) + "." + std::to_string(ip[2]) + "." +
                        std::to_string(ip[3]);

                // it's ok cuz port is always unsigned short type
                dest_port = static_cast<unsigned short>(request[8]) << 8 | static_cast<unsigned short>(request[9]);
                co_return tcp::endpoint{asio::ip::address_v4::from_string(ipStr), dest_port};
            case SOCKS5::DOMAINNAME:
                // TODO: resolve DNS
                break;
        }
        co_return tcp::endpoint{tcp::v6(), 0};

    }
}
