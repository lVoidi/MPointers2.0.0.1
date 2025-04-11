//
// Created by roarb on 08/04/2025.
//
// client_app.cpp - Aplicación cliente para interactuar con MPointers

#include "../mpointer/mpointer.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <functional> // Para std::function

// Clase para gestionar un menú interactivo
class Menu {
public:
    using CommandFunction = std::function<void()>;
    
    void addCommand(const std::string& command, const std::string& description, CommandFunction function) {
        commands_[command] = function;
        descriptions_[command] = description;
    }
    
    void showMenu() const {
        std::cout << "\nComandos disponibles:" << std::endl;
        std::cout << std::setfill('-') << std::setw(50) << "-" << std::endl;
        std::cout << std::setfill(' ');
        
        for (const auto& [cmd, desc] : descriptions_) {
            std::cout << std::setw(15) << std::left << cmd << " | " << desc << std::endl;
        }
        
        std::cout << std::setfill('-') << std::setw(50) << "-" << std::endl;
    }
    
    bool executeCommand(const std::string& input) {
        std::istringstream iss(input);
        std::string command;
        iss >> command;
        
        if (command == "exit" || command == "quit") {
            return false;
        }
        
        auto it = commands_.find(command);
        if (it != commands_.end()) {
            try {
                it->second();
            } catch (const std::exception& e) {
                std::cerr << "Error al ejecutar el comando: " << e.what() << std::endl;
            }
        } else {
            std::cout << "Comando desconocido: " << command << std::endl;
            showMenu();
        }
        
        return true;
    }
    
private:
    std::map<std::string, CommandFunction> commands_;
    std::map<std::string, std::string> descriptions_;
};

// Clase para gestionar la aplicación cliente
class ClientApp {
public:
    ClientApp() : currentId_(-1) {
        initializeMenu();
    }
    
    bool initialize(const std::string& host, int port) {
        try {
            std::cout << "Conectando al servidor en " << host << ":" << port << "..." << std::endl;
            MPointer<int>::Init(host, port);
            std::cout << "Conexión establecida correctamente" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error al conectar: " << e.what() << std::endl;
            return false;
        }
    }
    
    void run() {
        std::cout << "=== Cliente MPointers ===\n" << std::endl;
        menu_.showMenu();
        
        std::string input;
        do {
            std::cout << "\n> ";
            std::getline(std::cin, input);
        } while (menu_.executeCommand(input));
        
        std::cout << "Cerrando aplicación cliente..." << std::endl;
    }
    
private:
    Menu menu_;
    std::vector<MPointer<int>> pointers_;
    int currentId_;
    
    void initializeMenu() {
        menu_.addCommand("help", "Muestra este menú de ayuda", [this]() { menu_.showMenu(); });
        
        menu_.addCommand("create", "Crea un nuevo MPointer<int>", [this]() {
            try {
                MPointer<int> ptr = MPointer<int>::New();
                pointers_.push_back(ptr);
                std::cout << "Creado MPointer<int> con ID: " << &ptr << " en posición " << (pointers_.size() - 1) << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error al crear MPointer: " << e.what() << std::endl;
            }
        });
        
        menu_.addCommand("list", "Lista todos los MPointers creados", [this]() {
            if (pointers_.empty()) {
                std::cout << "No hay MPointers creados" << std::endl;
                return;
            }
            
            std::cout << "Lista de MPointers:" << std::endl;
            for (size_t i = 0; i < pointers_.size(); i++) {
                std::cout << "  [" << i << "] ID: " << &pointers_[i];
                
                try {
                    if (pointers_[i].isValid()) {
                        std::cout << ", Valor: " << *pointers_[i];
                    } else {
                        std::cout << " (inválido)";
                    }
                } catch (const std::exception&) {
                    std::cout << " (error al leer valor)";
                }
                
                std::cout << std::endl;
            }
        });
        
        menu_.addCommand("select", "Selecciona un MPointer por posición (select <posición>)", [this]() {
            int position;
            std::cout << "Ingrese la posición del MPointer: ";
            std::cin >> position;
            std::cin.ignore(); // Limpiar buffer
            
            if (position < 0 || position >= static_cast<int>(pointers_.size())) {
                std::cout << "Posición inválida" << std::endl;
                return;
            }
            
            currentId_ = position;
            std::cout << "Seleccionado MPointer en posición " << position << " con ID: " << &pointers_[position] << std::endl;
        });
        
        menu_.addCommand("get", "Obtiene el valor del MPointer seleccionado", [this]() {
            if (currentId_ < 0 || currentId_ >= static_cast<int>(pointers_.size())) {
                std::cout << "No hay MPointer seleccionado" << std::endl;
                return;
            }
            
            try {
                int value = *pointers_[currentId_];
                std::cout << "Valor del MPointer[" << currentId_ << "]: " << value << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error al obtener valor: " << e.what() << std::endl;
            }
        });
        
        menu_.addCommand("set", "Establece el valor del MPointer seleccionado (set <valor>)", [this]() {
            if (currentId_ < 0 || currentId_ >= static_cast<int>(pointers_.size())) {
                std::cout << "No hay MPointer seleccionado" << std::endl;
                return;
            }
            
            int value;
            std::cout << "Ingrese el nuevo valor: ";
            std::cin >> value;
            std::cin.ignore(); // Limpiar buffer
            
            try {
                *pointers_[currentId_] = value;
                std::cout << "Valor del MPointer[" << currentId_ << "] establecido a: " << value << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error al establecer valor: " << e.what() << std::endl;
            }
        });
        
        menu_.addCommand("copy", "Copia un MPointer a otro (copy <origen> <destino>)", [this]() {
            if (pointers_.size() < 2) {
                std::cout << "Se necesitan al menos dos MPointers para realizar una copia" << std::endl;
                return;
            }
            
            int source, dest;
            std::cout << "Ingrese la posición del MPointer origen: ";
            std::cin >> source;
            std::cout << "Ingrese la posición del MPointer destino: ";
            std::cin >> dest;
            std::cin.ignore(); // Limpiar buffer
            
            if (source < 0 || source >= static_cast<int>(pointers_.size()) ||
                dest < 0 || dest >= static_cast<int>(pointers_.size())) {
                std::cout << "Posición inválida" << std::endl;
                return;
            }
            
            try {
                pointers_[dest] = pointers_[source];
                std::cout << "MPointer[" << source << "] copiado a MPointer[" << dest << "]" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error al copiar MPointer: " << e.what() << std::endl;
            }
        });
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <port>" << std::endl;
        std::cerr << "Ejemplo: " << argv[0] << " localhost 9090" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);

    ClientApp app;
    if (!app.initialize(host, port)) {
        return 1;
    }
    
    app.run();
    return 0;
} 