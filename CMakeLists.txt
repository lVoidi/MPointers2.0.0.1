cmake_minimum_required(VERSION 3.30)
project(mpointers)

set(CMAKE_CXX_STANDARD 20)

add_executable(mpointers main.cpp
        protocol/message.h
        protocol/message.cpp
        memory_manager/main.cpp
        memory_manager/memory_manager.h
        memory_manager/memory_manager.cpp
        memory_manager/garbage_collector.h
        memory_manager/garbage_collector.cpp
        memory_manager/socket_server.h
        memory_manager/socket_server.cpp
        mpointer/mpointer.h
        mpointer/socket_client.h
        mpointer/socket_client.cpp
        examples/linked_list.h
        examples/linked_list.cpp
        examples/main.cpp)
