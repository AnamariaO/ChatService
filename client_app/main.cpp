#include "../shared/utils/signal_handler.h"
#include "../shared/config.h"
#include "client_app.h"
#include <iostream>

int main(int argc, char* argv[]) {
  std::string server_ip = DEFAULT_IP; // Localhost
  int port = DEFAULT_PORT;            // Match with server port

  //parsing arguments
  for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--ip" || arg == "-i") && i + 1 < argc) {
            server_ip = argv[++i];
        } else if ((arg == "--port" || arg == "-p") && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return 1;
        }
  }

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