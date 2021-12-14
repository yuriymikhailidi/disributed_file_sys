cmake_minimum_required(VERSION 3.20)
project(filesys C)

set(CMAKE_C_STANDARD 11)

add_executable(filesys
        client/dfclient.c
        server/dfserver.c)
