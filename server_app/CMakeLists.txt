add_executable(_server
    main.cpp
    server_app.cpp
)

target_include_directories(_server PRIVATE
    ${CMAKE_SOURCE_DIR}/shared/signal_handler
)