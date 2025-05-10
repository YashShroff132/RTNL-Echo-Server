cmake_minimum_required(VERSION 3.10)
project(HighConcurrencyTcpServer)

set(CMAKE_CXX_STANDARD 17)

# Boost is only needed for the client
find_package(Boost 1.70 REQUIRED system)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(src)

# ⚙️ New epoll-based server build
add_executable(server
    main.cpp
    src/epoll_server.cpp
)

# 🧠 Boost-based client remains the same
add_executable(client
    main_client.cpp
    src/TcpClient.cpp
)

target_link_libraries(client PRIVATE ${Boost_LIBRARIES})
