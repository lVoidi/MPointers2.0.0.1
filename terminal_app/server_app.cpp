//
// Created by roarb on 08/04/2025.
//
// server_app.cpp - Aplicación de servidor para MPointers

#include "../memory_manager/memory_manager.h"
#include "../memory_manager/garbage_collector.h"
#include "../memory_manager/socket_server.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>

// Solución multiplataforma para crear directorios
#ifdef _WIN32
#include <direct.h> // Para _mkdir en Windows
#define MKDIR(dir) _mkdir(dir)
#define MKDIR_ERROR EEXIST
#else
#include <sys/stat.h> // Para mkdir en sistemas POSIX
#include <errno.h>
#define MKDIR(dir) mkdir(dir, 0755)
#define MKDIR_ERROR EEXIST
#endif

std::atomic<bool> running(true);

// Manejador de señales para cerrar el servidor limpiamente
void signalHandler(int signum) {
    std::cout << "\nSeñal recibida (" << signum << "). Cerrando servidor..." << std::endl;
    running = false;
}

void printUsage(const char* programName) {
    std::cout << "Uso: " << programName
              << " --port PORT --memsize SIZE_MB --dumpFolder DUMP_FOLDER" << std::endl;
    std::cout << "\nEjemplo: " << programName << " --port 9090 --memsize 64 --dumpFolder dumps" << std::endl;
    std::cout << "\nParámetros:" << std::endl;
    std::cout << "  --port PORT         Puerto para escuchar conexiones (ej: 9090)" << std::endl;
    std::cout << "  --memsize SIZE_MB   Tamaño de memoria a reservar en MB (ej: 64)" << std::endl;
    std::cout << "  --dumpFolder PATH   Carpeta para guardar archivos de volcado de memoria" << std::endl;
}

void printMemoryStatus(MemoryManager* memoryManager) {
    std::cout << "\n--- Estado de la Memoria ---" << std::endl;
    
    // Llamamos directamente a dumpMemoryState que ya hace parte de esta funcionalidad
    memoryManager->dumpMemoryState();
    
    // Nota: En una implementación completa, MemoryManager debería proporcionar
    // métodos públicos para obtener estadísticas como getUsedMemory(), getTotalMemory(), etc.
    std::cout << "----------------------------\n" << std::endl;
}

// Función para crear directorio de manera portable
bool createDirectory(const std::string& path) {
    int result = MKDIR(path.c_str());
    return (result == 0 || errno == MKDIR_ERROR);
}

int main(int argc, char* argv[]) {
    std::cout << "=== Servidor MPointers - Memory Manager ===\n" << std::endl;
    
    // Registrar manejador de señales
    std::signal(SIGINT, signalHandler);  // Ctrl+C
    
    // Parse command line arguments
    int port = 0;
    size_t memsize = 0;
    std::string dumpFolder;

    // Simple argument parsing
    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i];
        if (i + 1 >= argc) {
            std::cerr << "Error: Falta valor para " << arg << std::endl;
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
            std::cerr << "Error: Argumento desconocido: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // Validate arguments
    if (port <= 0 || memsize <= 0 || dumpFolder.empty()) {
        std::cerr << "Error: Argumentos inválidos" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    // Create dump folder if it doesn't exist
    if (!createDirectory(dumpFolder)) {
        std::cerr << "Error al crear carpeta de volcado: " << dumpFolder << std::endl;
        return 1;
    }

    try {
        std::cout << "Iniciando Memory Manager con " << memsize << "MB de memoria..." << std::endl;
        // Initialize memory manager
        MemoryManager memoryManager(memsize, dumpFolder);

        std::cout << "Iniciando recolector de basura..." << std::endl;
        // Start garbage collector
        GarbageCollector garbageCollector(&memoryManager);
        garbageCollector.start();

        std::cout << "Iniciando servidor de sockets en puerto " << port << "..." << std::endl;
        // Start socket server
        SocketServer server(port, &memoryManager);
        server.start();

        std::cout << "\nServidor ejecutándose. Presione Ctrl+C para finalizar.\n" << std::endl;
        std::cout << "Comandos disponibles:" << std::endl;
        std::cout << "  status - Muestra el estado actual de la memoria" << std::endl;
        std::cout << "  gc - Ejecuta el garbage collector manualmente" << std::endl;
        std::cout << "  exit - Cierra el servidor" << std::endl;
        
        // Bucle principal de comando
        std::string command;
        while (running) {
            std::cout << "> ";
            std::getline(std::cin, command);
            
            if (command == "status") {
                printMemoryStatus(&memoryManager);
            } else if (command == "gc") {
                std::cout << "Ejecutando garbage collector manualmente..." << std::endl;
                memoryManager.startGarbageCollector();
            } else if (command == "exit") {
                std::cout << "Cerrando servidor..." << std::endl;
                break;
            } else if (!command.empty()) {
                std::cout << "Comando desconocido. Opciones: status, gc, exit" << std::endl;
            }
        }

        // Cleanup
        std::cout << "Deteniendo servicios..." << std::endl;
        server.stop();
        garbageCollector.stop();
        std::cout << "Servidor finalizado correctamente." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 