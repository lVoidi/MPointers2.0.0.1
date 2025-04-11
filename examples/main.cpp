//
// Created by roarb on 08/04/2025.
//

#include <iostream>
#include <string>
#include "linked_list.h"
#include "../mpointer/mpointer.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);

    try {
        // Inicializar la conexión usando el método estático centralizado
        // MPointer<int>::Init(host, port);
        MPointerConnection::Init(host, port);
        std::cout << "Conexión establecida correctamente" << std::endl;

        // Crear una lista enlazada de enteros
        std::cout << "\n-- Creando lista enlazada de enteros --" << std::endl;
        LinkedList<int> intList;
        
        // Añadir elementos a la lista
        std::cout << "Añadiendo elementos a la lista..." << std::endl;
        for (int i = 1; i <= 5; i++) {
            intList.pushBack(i * 10);
        }
        intList.print();
        
        // Insertar al inicio
        std::cout << "\nInsertando elementos al inicio..." << std::endl;
        intList.pushFront(5);
        intList.pushFront(2);
        intList.print();
        
        // Acceder a un elemento específico
        std::cout << "\nAccediendo al elemento en posición 3: " << intList.get(3) << std::endl;
        
        // Eliminar un elemento
        std::cout << "\nEliminando el elemento en posición 2..." << std::endl;
        intList.remove(2);
        intList.print();
        
        // Eliminar el primer elemento
        std::cout << "\nEliminando el primer elemento..." << std::endl;
        intList.remove(0);
        intList.print();
        
        // Limpiar la lista
        std::cout << "\nLimpiando la lista..." << std::endl;
        intList.clear();
        std::cout << "Tamaño de la lista después de limpiar: " << intList.size() << std::endl;
        intList.print();
        
        // Demostrar funcionamiento con otro tipo de datos
        std::cout << "\n-- Creando lista enlazada de cadenas --" << std::endl;
        LinkedList<std::string> stringList;
        stringList.pushBack("Hola");
        stringList.pushBack("Mundo");
        stringList.pushBack("con");
        stringList.pushBack("MPointers");
        stringList.print();
        
        std::cout << "\nPrograma completado exitosamente" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
