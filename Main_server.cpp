#include "Main_server.hpp"

namespace SOCKS5 {

    namespace asio = boost::asio;
    using tcp = asio::ip::tcp;


    Main_server::Main_server(const unsigned short port_) : port(port_) {}

    void Main_server::start_server() {
        asio::io_context ioContext;
        co_spawn(ioContext, [this] { return start(); }, asio::detached);
        ioContext.run();
    }

    asio::awaitable<void> Main_server::start() {
        const auto executor = co_await asio::this_coro::executor;

        tcp::acceptor acceptor{executor, {tcp::v4(), this->port}};

        for (;;) {
            tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
            co_spawn(executor, [this, &socket] { return client_handler(std::move(socket)); }, asio::detached);

        }

    }

    asio::awaitable<void> Main_server::client_handler(tcp::socket client_socket) {

        std::array<unsigned char, 3> welcome_message{};

        if (auto [ec, n] = co_await client_socket.async_read_some(asio::buffer(welcome_message),
                                                              as_tuple(asio::use_awaitable)); ec) {
            std::cerr << ec.message() << std::endl;
            client_socket.close();
            co_return;
        }
        if (bool is_valid = co_await this->validate_welcome_message(welcome_message); !is_valid) {
            client_socket.close();
            co_return;
        }
        co_await this->send_auth_response(client_socket, select_auth_method(welcome_message));

        auto client_req = co_await this->get_client_request(client_socket);
        tcp::endpoint endpoint = co_await this->handle_client_request(client_req);

        tcp::socket remote_socket{co_await asio::this_coro::executor};
        if (auto [error] = co_await remote_socket.async_connect(endpoint, as_tuple(asio::use_awaitable)); !error) {

            // it it takes 0x00 to be an int, so we can't use std::array
            // ReSharper disable once CppTemplateArgumentsCanBeDeduced
            std::array<unsigned char, 10> reply{
                VERSION, 0x00, RSV, IP_V4, 0x7f, 0x00, 0x00,
                                                0x01, 0x00, 0x00};
            reply[1] = get_connect_error(client_req[1], error);
            reply[8] = this->port >> 8 & 0xff;
            reply[9] = this->port & 0xff;
            if ( auto [er, bytes] = co_await client_socket.async_send(asio::buffer(reply),
                                                                 as_tuple(asio::use_awaitable));er) {
                std::cerr << er.message() << std::endl;
                client_socket.close();
                co_return;
            }
            const auto executor = co_await asio::this_coro::executor;
            co_spawn(executor, [this, &client_socket, &remote_socket] {
                return handle_client_data(std::move(client_socket), std::move(remote_socket));
            }, asio::detached);
        } else {
            std::cerr << error.what() << std::endl;
        }
        co_return;
    }

    asio::awaitable<bool> Main_server::validate_welcome_message(const std::array<unsigned char, 3> &welcome_message) {
        if (!(welcome_message[0] == VERSION && welcome_message[2] == NO_AUTH)) {
            co_return false;
        }
        co_return true;
    }


    unsigned char Main_server::select_auth_method(const std::array<unsigned char, 3> &welcome_message) {
        const unsigned char nmethods = welcome_message[1];
        for (unsigned char i = 2; i < nmethods + 2; ++i) {
            if (welcome_message[i] == NO_AUTH) {
                return welcome_message[i];
            }
        }
        return NO_ACCEPTABLE_METHODS;
    }


    asio::awaitable<void> Main_server::send_auth_response(tcp::socket &socket, const unsigned char &method) {
        std::array response{VERSION, method};
        co_await async_write(socket, asio::buffer(response), asio::use_awaitable);
        co_return;
    }

    asio::awaitable<std::vector<unsigned char>> Main_server::get_client_request(tcp::socket &socket) {
        asio::streambuf buffer;
        co_await async_read(socket, buffer, asio::transfer_at_least(1), asio::use_awaitable);
        std::vector<unsigned char> request(buffers_begin(buffer.data()), buffers_end(buffer.data()));
        co_return request;

    }

    asio::awaitable<tcp::endpoint> Main_server::handle_client_request(std::vector<unsigned char> &request) {
        if (auto cmd = request[1]; cmd != CONNECT) {
            std::cerr << "Unsupported operation: " << static_cast<int>(cmd) << std::endl;
            co_return tcp::endpoint{tcp::v6(), 0};
        }

        unsigned short dest_port;
        std::array<unsigned char, 4> ip{};
        std::string ipStr;
        switch (request[3]) {
            case IP_V4:
                std::copy(request.data() + 4, request.data() + 8, ip.data());
                ipStr =
                        std::to_string(ip[0]) + "." + std::to_string(ip[1]) + "." + std::to_string(ip[2]) + "." +
                        std::to_string(ip[3]);

                dest_port = static_cast<unsigned short>(request[8]) << 8 | static_cast<unsigned short>(request[9]);
                co_return tcp::endpoint{asio::ip::address_v4::from_string(ipStr), dest_port};
            case DOMAIN_NAME: {
                int domain_name_len = request[4];
                std::vector<char> host_name(domain_name_len);
                int i = 0;
                for (i = 0; i < domain_name_len; ++i) {
                    host_name[i] = static_cast<char>(request[i + 5]);
                }
                dest_port = static_cast<unsigned short>(request[i + 5]) << 8 | static_cast<unsigned short>(request[i + 6]);

                if (resolved_hosts[std::string(host_name.data(), domain_name_len)] != asio::ip::make_address_v4("0.0.0.0")) {
                    co_return asio::ip::tcp::endpoint {this->resolved_hosts[std::string(host_name.data(), domain_name_len)], dest_port};
                }
                asio::ip::tcp::resolver resolver{co_await asio::this_coro::executor};
                asio::ip::tcp::resolver::query query{std::string(host_name.data(), domain_name_len),
                    std::to_string(dest_port)};
                std::cout << std::string(host_name.data(), domain_name_len) << ' ' << dest_port << std::endl;
                if (auto [ec, it] = co_await resolver.async_resolve(query, as_tuple(asio::use_awaitable)); !ec) {
                    asio::ip::tcp::endpoint endpoint = *it;
                    resolved_hosts[std::string(host_name.data(), domain_name_len)] = endpoint.address().to_v4();
                    co_return *it;
                }
            }
            default:
                    co_return tcp::endpoint{tcp::v6(), 0};
        }
        co_return tcp::endpoint{tcp::v6(), 0};

    }

    unsigned char Main_server::get_connect_error(const unsigned char &cmd, const boost::system::error_code &code) {
        if (code == asio::error::network_unreachable) {
            return NETWORK_UNREACHABLE;
        }  if (code == asio::error::host_unreachable) {
            return HOST_UNREACHABLE;
        }  if (code == asio::error::connection_refused) {
            return CONNECTION_REFUSED;
        }  if (code == asio::error::timed_out) {
            return TTL_EXPIRED;
        }  if (cmd != CONNECT) {
            return COMMAND_NOT_SUPPORTED;
        }  if (code == asio::error::address_family_not_supported) {
            return ADDRESS_TYPE_NOT_SUPPORTED;
        }
        return SUCCESS;
    }

    // we cannot use unique_ptr in coroutines
    asio::awaitable<void> Main_server::handle_client_data(tcp::socket client_socket, tcp::socket remote_socket) const {
        auto cs = std::make_shared<tcp::socket>(std::move(client_socket));
        auto rs = std::make_shared<tcp::socket>(std::move(remote_socket));
        auto executor = co_await asio::this_coro::executor;
        co_spawn(executor, [this, cs, rs] {
                return client_to_remote(cs, rs);
            }, asio::detached);
        co_spawn(executor, [this, cs, rs] {
                return remote_to_client(rs, cs);
            }, asio::detached);
        co_return;
    }



    asio::awaitable<void> Main_server::client_to_remote(const std::shared_ptr<tcp::socket>& client_socket, const std::shared_ptr<tcp::socket>& remote_socket) {
        for(;;) {
            co_await client_socket->async_wait(asio::socket_base::wait_read, asio::use_awaitable);
            std::array<unsigned char, 4096> buffer{};
            std::size_t bytes = co_await client_socket->async_read_some(asio::buffer(buffer), asio::use_awaitable);
            bytes = co_await remote_socket->async_write_some(asio::buffer(buffer, bytes), asio::use_awaitable);
        }
    }

     auto Main_server::remote_to_client(const std::shared_ptr<tcp::socket>&remote_socket,
                                       const std::shared_ptr<tcp::socket>&client_socket) -> asio::awaitable<void> {
        for(;;) {
            co_await remote_socket->async_wait(asio::socket_base::wait_read, asio::use_awaitable);
            std::array<unsigned char, 4096> buffer{};
            const std::size_t bytes = co_await remote_socket->async_read_some(asio::buffer(buffer), asio::use_awaitable);
            co_await client_socket->async_write_some(asio::buffer(buffer, bytes), asio::use_awaitable);
        }
    }

}
