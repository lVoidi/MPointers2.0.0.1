cmake_minimum_required(VERSION 3.20)
project(mpointers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Definiciones específicas para Windows para silenciar advertencias
if(MSVC)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
    )
endif()

# Bibliotecas principales
add_library(protocol
        protocol/message.h
        protocol/message.cpp
)

add_library(socket_client
        mpointer/socket_client.h
        mpointer/socket_client.cpp
)
target_link_libraries(socket_client protocol)

# Para Windows, enlazamos con la biblioteca de sockets
if(WIN32)
    target_link_libraries(socket_client wsock32 ws2_32)
endif()

# Memory Manager
add_executable(memory_manager
        memory_manager/main.cpp
        memory_manager/memory_manager.h
        memory_manager/memory_manager.cpp
        memory_manager/garbage_collector.h
        memory_manager/garbage_collector.cpp
        memory_manager/socket_server.h
        memory_manager/socket_server.cpp
)
target_link_libraries(memory_manager protocol)
if(WIN32)
    target_link_libraries(memory_manager wsock32 ws2_32)
endif()

# Ejemplos
add_executable(examples
        examples/main.cpp
        examples/linked_list.h
        examples/linked_list.cpp
        mpointer/mpointer.h
)
target_link_libraries(examples socket_client)

# Tests
add_executable(mpointer_test
        tests/mpointer_test.cpp
)
target_link_libraries(mpointer_test socket_client)

# Aplicación cliente-servidor de terminal
add_executable(server_app
        terminal_app/server_app.cpp
        memory_manager/memory_manager.cpp
        memory_manager/garbage_collector.cpp
        memory_manager/socket_server.cpp
)
target_link_libraries(server_app protocol)
if(WIN32)
    target_link_libraries(server_app wsock32 ws2_32)
endif()

add_executable(client_app
        terminal_app/client_app.cpp
)
target_link_libraries(client_app socket_client)

# Archivo principal
add_executable(mpointers main.cpp)
target_link_libraries(mpointers socket_client)

# Configurar pruebas
enable_testing()
add_test(NAME MPointerBasicTest COMMAND mpointer_test localhost 9090)

# Objetivo para ejecutar todas las pruebas de una vez
add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS mpointer_test
        COMMENT "Ejecutando todas las pruebas"
)