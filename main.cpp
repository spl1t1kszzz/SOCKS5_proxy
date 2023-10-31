#include <iostream>
#include "Main_server.hpp"


using namespace std;

int main(int argc, char* argv[]) {
    if (argc  < 2) {
        cerr << "Not enough args" << endl;
        return EXIT_FAILURE;
    } else {
        unsigned short port = atoi(argv[1]);
        SOCKS5::Main_server server(port);
        server.start_server();
    }

    return 0;
}