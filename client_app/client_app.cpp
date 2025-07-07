#include "client_app.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Client::Client(const std::string &server_ip, int port)
    : server_ip(server_ip), port(port), client_fd(-1), running(false) {}

Client::~Client() { stop(); }

void Client::setUser(const std::string &_user) { user = _user; }

bool Client::connectToServer() {
  client_fd = socket(AF_INET, SOCK_STREAM, 0); // creates the socket
  if (client_fd == -1) {
    std::cerr << "Error creating socket\n";
    return false;
  }

  sockaddr_in server_addr{}; // Prepares a (sockaddr_in)struct with the server
                             // IP and port.
  server_addr.sin_family = AF_INET;
  server_addr.sin_port =
      htons(port); // converts port number from host byte order to network byte
                   // order (big endian)

  if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <=
      0) { // This line converts a human-readable IP address string (like
           // "127.0.0.1") into a binary format that the socket API can
           // understand and stores it in server_addr.sin_addr
    std::cerr << "Invalid server address\n"; // 0 -invalid ip/ -1 -system error
    return false;
  }

  if (connect(client_fd, (sockaddr *)&server_addr, sizeof(server_addr)) <
      0) { // attempts to establish a TCP connection to the server you specified
           // (by IP and port).
    std::cerr << "Connection failed\n"; // If the server is not listening,
                                        // unreachable, or wrong port
    return false;
  }

  std::cout << "Connected to server at " << server_ip << ":" << port
            << std::endl;
  return true;
}

void Client::start() {
  running = true;
  receiver_thread = std::thread(
      &Client::receiveMessages,
      this); // allows the client to listen for messages from the server in the
             // background, while the main thread keeps reading user input

  std::string message;
  while (std::getline(std::cin, message)) {
    if (!running)
      break;
    if (message == "/quit") {
      stop();
      break;
    }

    message = user + ": " + message;
    ssize_t bytes_sent =
        send(client_fd, message.c_str(), message.length(),
             0); // sends message to server using the sovket API, transmits raw
                 // bytes over TCP connection
    if (bytes_sent == -1) {
      std::cerr << "Failed to send message.\n";
    }
  }
}

void Client::stop() {

  // if(running){
  running = false;
  shutdown(client_fd, SHUT_RDWR);
  close(client_fd);
  if (receiver_thread.joinable()) {
    receiver_thread.join();
  }
  std::cout << "Disconnected from server.\n";
  // }
}

void Client::receiveMessages() {
  char buffer[1024];
  while (running) {
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
      std::cout << "Server closed the connection.\n";
      running = false;
      break;
    }
    buffer[bytes_received] = '\0';
    std::cout << "[Server] from " << buffer << std::endl;
  }
}