add_executable(_client
    main.cpp
    client_app.cpp
)

target_include_directories(_client PRIVATE
    ${CMAKE_SOURCE_DIR}/shared
)