#include <iostream>
#include "Main_server.hpp"

using namespace std;

int main(int argc, char **argv) {
    setlocale(LC_ALL, "Russian");
    if (argc  < 2) {
        cerr << "Not enough args" << endl;
        return EXIT_FAILURE;
    } else {
        unsigned short port = static_cast<unsigned short>(strtol(argv[1], nullptr, 10));
        SOCKS5::Main_server server(port);
        server.start_server();
    }

    return 0;
}