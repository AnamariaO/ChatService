#pragma once

#include <atomic>
#include <string>
#include <thread>

class Client {
public:
  Client(const std::string &server_ip, int port); // contructor initializing ip and port
  ~Client();

  void setUser(const std::string &_user);
  bool connectToServer(); // try to connect to server
  void start();           // start sending/recieving
  void stop();

private:
  std::string user;

  void receiveMessages();

  std::string server_ip;
  int port;
  int client_fd; // client socket  descriptor
  std::atomic<bool> running;
  std::thread receiver_thread;
};