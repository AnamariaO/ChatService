#include "client_app.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

Client::Client(const std::string& server_ip, int port)
    : server_ip(server_ip), port(port), server_fd(-1), running(false) {}

Client::~Client() {
    stop();
}

void Client::setUser(const std::string& _user)
{
    user = _user;
}

bool Client::connectToServer() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Error creating socket\n";
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid server address\n";
        return false;
    }

    if (connect(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed\n";
        return false;
    }

    std::cout << "Connected to server at " << server_ip << ":" << port << std::endl;
    return true;
}

void Client::start() {
    running = true;
    receiver_thread = std::thread(&Client::receiveMessages, this);

    std::string message;
    while (running && std::getline(std::cin, message)) {
        if (message == "/quit") {
            stop();
            break;
        }
    
    message = user + ": " +message;
        ssize_t bytes_sent = send(server_fd, message.c_str(), message.length(), 0);
        if (bytes_sent == -1) {
            std::cerr << "Failed to send message.\n";
        }
    }
}

void Client::stop() {
    if (running) {
        running = false;
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
        if (receiver_thread.joinable()) {
            receiver_thread.join();
        }
        std::cout << "Disconnected from server.\n";
    }
}

void Client::receiveMessages() {
    char buffer[1024];
    while (running) {
        ssize_t bytes_received = recv(server_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            running = false;
            std::cout << "Server closed the connection.\n";
            break;
        }

        buffer[bytes_received] = '\0';
        std::cout << "[Server] from " << buffer << std::endl;
    }
}