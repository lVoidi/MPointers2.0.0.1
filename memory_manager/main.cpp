//
// Created by roarb on 08/04/2025.
//
// main.cpp
#include "memory_manager.h"
#include "garbage_collector.h"
#include "socket_server.h"
#include <iostream>
#include <string>
#include <filesystem>


void printUsage(const char* programName) {
    std::cout << "Usage: " << programName
              << " --port PORT --memsize SIZE_MB --dumpFolder DUMP_FOLDER" << std::endl;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    int port = 0;
    size_t memsize = 0;
    std::string dumpFolder;

    // Simple argument parsing
    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i];
        if (i + 1 >= argc) {
            std::cerr << "Missing value for " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }

        if (arg == "--port") {
            port = std::stoi(argv[i + 1]);
        } else if (arg == "--memsize") {
            memsize = std::stoi(argv[i + 1]);
        } else if (arg == "--dumpFolder") {
            dumpFolder = argv[i + 1];
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // Validate arguments
    if (port <= 0 || memsize <= 0 || dumpFolder.empty()) {
        std::cerr << "Invalid arguments" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    // Create dump folder if it doesn't exist
    std::filesystem::create_directories(dumpFolder);

    try {
        // Initialize memory manager
        MemoryManager memoryManager(memsize, dumpFolder);

        // Start garbage collector
        GarbageCollector garbageCollector(&memoryManager);
        garbageCollector.start();

        // Start socket server
        SocketServer server(port, &memoryManager);
        server.start();

        std::cout << "Memory Manager running on port " << port << std::endl;
        std::cout << "Press Enter to quit..." << std::endl;
        std::cin.get();

        // Cleanup
        server.stop();
        garbageCollector.stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}