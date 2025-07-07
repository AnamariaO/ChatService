#include "../shared/signal_handler/signal_handler.h"
#include "../shared/config.h"
#include "client_app.h"
#include <iostream>

int main() {
  std::string server_ip = DEFAULT_IP; // Localhost
  int port = DEFAULT_PORT;            // Match with server port

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