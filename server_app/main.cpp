#include "server_app.h"
#include <iostream>

int main() {
    int port = 8080;

    Server server(port);
    server.start();

    std::cout << "[Server] Press Enter to stop the server...\n";
    std::cin.get(); // Wait for user input to stop the server

    server.stop();

    return 0;
}