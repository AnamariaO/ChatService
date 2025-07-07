#include "../shared/signal_handler/signal_handler.h"
#include "server_app.h"
#include <iostream>

int main() {
  int port = 8080;

  Server server(port);
  SignalHandler<Server>::registerHandler(&server, &Server::stop);
  server.start();

  while (server.isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
  // std::cout << "[Server] Press Enter to stop the server...\n";
  // std::cin.get(); // Wait for user input to stop the server

  server.stop();

  return 0;
}