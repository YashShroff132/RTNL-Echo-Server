cmake_minimum_required(VERSION 3.10)
project(HighConcurrencyTcpServer)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.70 REQUIRED system)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(src)

add_executable(server
    main.cpp
    src/TcpServer.cpp
    src/ClientSession.cpp
)

target_link_libraries(server PRIVATE ${Boost_LIBRARIES})

add_executable(client
    main_client.cpp
    src/TcpClient.cpp
)

target_link_libraries(client PRIVATE ${Boost_LIBRARIES})