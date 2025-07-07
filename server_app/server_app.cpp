#include "../shared/utils/signal_handler.h"
#include "../shared/utils/time_stamp.h"
#include "../shared/config.h"
#include "server_app.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(int port)
    : port_(port), running(false) {
} // sets up the port and marks the server as “not running yet.”

Server::~Server() { stop(); }

void Server::start() {
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("Socket failure: ");
    exit(EXIT_FAILURE);
  }

  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    exit(EXIT_FAILURE);
  }

  sockaddr_in server_address{}; // struct that defines the server’s address (IP
                                // + port + family)
  server_address.sin_family =
      AF_INET; // Set the address family to IPv4, matched with the socket() call
               // earlier.
  server_address.sin_port =
      htons(port_); // Host TO Network Short, Converts from the machine’s byte
                    // order to network byte order (big-endian), which is
                    // standard for sending data over the network.
  server_address.sin_addr.s_addr =
      INADDR_ANY; // special constant provided by the OS | It accepts
                  // connections from: Localhost/LAN/External network (if port
                  // is open)/Any IP/interface on the machine

  if (bind(server_fd, (sockaddr *)&server_address, sizeof(server_address)) <
      0) { // This socket is for receiving connections on this IP/port
    perror("Binding failure: ");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 5) <
      0) { // Start listening for incoming connections on this socket, 5 is the
           // backlog->how many connections can wait in the queue before they’re
           // accepted.
    perror("Listening failure: ");
    exit(EXIT_FAILURE);
  }

  running =
      true; // Sets the running flag to true, this flag controls the loop in
            // your acceptClients() thread, so it knows the server is live
  std::cout << "[Server] Listening on port " << port_ << "...\n";

  std::thread(&Server::acceptClients, this)
      .detach(); // Launches a new thread, Runs the acceptClients() method on
                 // the current Server object, Immediately detaches that thread
                 // (lets it run on its own, in the background)
  std::thread(&Server::handleServerCommands, this).detach();
}

void Server::handleServerCommands(){
    std::string input;

    while(running){
        std::getline(std::cin, input);

        if(input=="/exit"){
            std::cout << "[Server] Shutting down...\n";
            stop();
            break;
        }
        else if (input == "/list") {
            std::lock_guard<std::mutex> lock(clients_mutex);
            std::cout << "[Server] Connected clients: \n";
            if (client_fds.empty()) {
                std::cout << "(none)\n";
            } else {
                for (int fd : client_fds) {
                    std::cout <<"Client with fd: "<< fd << " connected."<<std::endl;
                }
                std::cout << std::endl;
            }
        } else if (!input.empty()) {
            std::cout << "[Server] Unknown command: " << input << "\n";

        }
    }
}

void Server::stop() {

  if (!running) //  avoid running shutdown steps again
    return;

  running = false;
  // cleanup client sockets
  {
    std::lock_guard<std::mutex> lock(
        clients_mutex); // creates a scoped lock on the clients_mutex to safely
                        // access shared data (client_fds). This ensures no
                        // other thread can modify client_fds while we’re
                        // iterating over it (thread safety).
    for (int client_fd :
         client_fds) {  // Loops over all connected client sockets (client_fd)
                        // and closes each one using the Linux close() system
                        // call.
      close(client_fd); // This terminates the connection with each client
    }
    client_fds.clear(); // Clears the list of client file descriptors. After we
                        // close them, they are no longer valid or needed.
                        // clear() removes all elements from the vector.
  } // lock_guard automatically releases the mutex when it goes out of scope
    // here

  // Close the server socket
  if (server_fd != -1) { // check if the server socket is open.
    close(server_fd);    // stops the server from accepting new connections.
    server_fd = -1;      // ignal that server is closed closed (good practice to
                         // prevent accidental reuse).
  }
}

void Server::acceptClients() {
  while (running) {
    sockaddr_in client_address;
    socklen_t client_length =
        sizeof(client_address); // size of the client_address buffer
    int client_fd =
        accept(server_fd, (sockaddr *)&client_address,
               &client_length); // waiting for new client connections on the
                                // lisening socket(server_fd)
    if (client_fd >= 0) {
      logClientStatus(client_fd, true);
      std::lock_guard<std::mutex> lock(clients_mutex);
      client_fds.push_back(
          client_fd); // add new client's descriptor to the list of clients
      std::thread(&Server::handleClient, this, client_fd).detach();
    }
  }
}

void Server::handleClient(int client_fd) {
  char buffer[BUFFER_SIZE];
  std::string user;
  std::string content;

  while (running) {
    ssize_t recieved_bytes =
        recv(client_fd, buffer, sizeof(buffer) - 1,
             0); // gett data from client (message stored in buffer/ number of
                 // bytes stored in recieved_bytes)
    if (recieved_bytes <= 0) {
      break; // connection is closed or other error occured
    }
    buffer[recieved_bytes] = '\0'; // null terminated string

    std::string new_message = buffer;
    size_t delimiter_pos = new_message.find(':');
    user = new_message.substr(0, delimiter_pos);
    content = new_message.substr(delimiter_pos + 1);
    std::cout << currentTimestamp() << "[User " << user << "]: " << content << std::endl;

    // Broadcast message to all other clients
    std::string timestamped_message = currentTimestamp() + " " + new_message;
    broadcastMessage(timestamped_message, client_fd);
  }

  close(client_fd); // Shuts down this client’s socket — it's done
                    // communicating.

  {
    std::lock_guard<std::mutex> lock(clients_mutex);
    client_fds.erase(
        std::remove(client_fds.begin(), client_fds.end(), client_fd),
        client_fds.end());
  }

  logClientStatus(client_fd, false);
}

void Server::broadcastMessage(std::string_view message, int sender_fd) {
  std::lock_guard<std::mutex> lock(clients_mutex);

  for (int cl_fd : client_fds) {
    if (cl_fd != sender_fd) {
      ssize_t bytes_sent = send(cl_fd, message.data(), message.length(), 0);
      if (bytes_sent == -1) {
        std::cerr << "Failed to send message to client " << cl_fd << std::endl;
      }
    }
  }
}

void Server::logClientStatus(int fd, bool connected) {
    std::cout << "Client " << fd << " is now " << (connected ? "connected" : "disconnected") << ".\n";
}
