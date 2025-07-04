cmake_minimum_required(VERSION 3.14)
project(ChatService LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(server_app)
add_subdirectory(client_app)