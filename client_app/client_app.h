#pragma once

#include <string>
#include <thread>
#include <atomic>

class Client {
public:
    Client(const std::string& host, int port);
    ~Client();

    void setUser(const std::string& _user);
    bool connectToServer();
    void start();
    void stop();

private:
    std::string user;

    void receiveMessages();

    std::string server_ip;
    int port;
    int server_fd;
    std::atomic<bool> running;
    std::thread receiver_thread;
};