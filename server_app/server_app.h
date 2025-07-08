#pragma once

#include <atomic> 
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Server {
public:
  explicit Server(int port); 
  ~Server();                 

  void start();
  void stop();

  bool isRunning() const { return running; } //needed for main

private:
  int server_fd; // socket descriptor
  int port_;     // tells the operating system which program or service on the
                 // machine should handle the incoming network connection
  std::atomic<bool> running; // thread safe flag to check if server is active

  std::vector<int> client_fds;
  std::mutex clients_mutex; // synchronizes access to client_fds and prevents
                            // crashes if two threads modify it at the same time
  
  void handleServerCommands();

  void acceptClients();
  void handleClient(int client_fd);
  void broadcastMessage(std::string_view message, int sender_fd);

  void logClientStatus(int fd, bool connected);
};