cmake_minimum_required(VERSION 3.8)
project(sysopki)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wno-unused-variable -ggdb")
set(CMAKE_BUILD_TYPE Debug)

set(Headers ./zad1/common.h)

add_executable(server zad1/server.c ${Headers})
add_executable(client zad1/client.c ${Headers})

target_link_libraries(server pthread)
target_link_libraries(client pthread)

