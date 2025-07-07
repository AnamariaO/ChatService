#include "../shared/signal_handler/signal_handler.h"
#include "client_app.h"
#include <iostream>

int main() {
  std::string server_ip = "127.0.0.1"; // Localhost
  int port = 8080;                     // Match with server port

  std::string name;
  std::cout << "Enter user: ";
  std::getline(std::cin, name);

  Client client(server_ip, port);
  client.setUser(name);

  SignalHandler<Client>::registerHandler(&client, &Client::stop);

  if (!client.connectToServer()) {
    std::cerr << "Failed to connect to server." << std::endl;
    return 1;
  }

  client.start();

  return 0;
}