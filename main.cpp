#include <iostream>
#include "Main_server.hpp"


int main(const int argc, char **argv) {
    setlocale(LC_ALL, "Russian");
    if (argc  < 2) {
        std::cerr << "Not enough args" << std::endl;
        return EXIT_FAILURE;
    }
    const auto port = static_cast<unsigned short>(strtol(argv[1], nullptr, 10));
    SOCKS5::Main_server server(port);
    server.start_server();
    return 0;
}
