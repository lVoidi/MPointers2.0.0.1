//
// Created by roarb on 08/04/2025.
//

#include <iostream>

int main() {
    std::cout << "MPointers 2.0 - Sistema de Punteros Remotos" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "Este es el archivo principal del proyecto." << std::endl;
    std::cout << "Para ejecutar cada componente, use los siguientes comandos:" << std::endl;
    std::cout << std::endl;
    
    std::cout << "1. Iniciar el servidor (Memory Manager):" << std::endl;
    std::cout << "   ./memory_manager --port 9090 --memsize 64 --dumpFolder dumps" << std::endl;
    std::cout << std::endl;
    
    std::cout << "2. Aplicación cliente-servidor interactiva:" << std::endl;
    std::cout << "   Servidor: ./server_app --port 9090 --memsize 64 --dumpFolder dumps" << std::endl;
    std::cout << "   Cliente:  ./client_app localhost 9090" << std::endl;
    std::cout << std::endl;
    
    std::cout << "3. Ejecutar las pruebas básicas:" << std::endl;
    std::cout << "   ./mpointer_test localhost 9090" << std::endl;
    std::cout << std::endl;
    
    std::cout << "4. Ejecutar el ejemplo de lista enlazada:" << std::endl;
    std::cout << "   ./examples localhost 9090" << std::endl;
    std::cout << std::endl;
    
    return 0;
}