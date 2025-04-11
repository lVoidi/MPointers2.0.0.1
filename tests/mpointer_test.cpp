//
// Created by roarb on 09/04/2025.
//
//
// Created by roarb on 08/04/2025.
//

#include <iostream>
#include "../mpointer/mpointer.h"
#include "../examples/linked_list.h" // Incluir la lista enlazada
#include <vector>
#include <cassert> // Para aserciones
#include <stdexcept> // Para std::runtime_error

// --- Pruebas Básicas Existentes (Asumo que quieres mantenerlas) ---
void test_basic_operations() {
    std::cout << "Ejecutando prueba básica de MPointer..." << std::endl;
    MPointer<int> ptr1 = MPointer<int>::New();
    assert(ptr1.isValid());
    std::cout << "Ptr1 creado con ID: " << &ptr1 << std::endl;

    *ptr1 = 42;
    assert(*ptr1 == 42);
    std::cout << "Valor asignado a ptr1: " << *ptr1 << std::endl;

    MPointer<int> ptr2;
    ptr2 = ptr1;
    assert(&ptr2 == &ptr1);
    std::cout << "Ptr2 asignado desde ptr1, ID: " << &ptr2 << std::endl;

    assert(*ptr2 == 42);
    *ptr2 = 99;
    assert(*ptr1 == 99);
    std::cout << "Valor modificado a través de ptr2: " << *ptr1 << std::endl;

    MPointer<int> ptr3 = MPointer<int>::New();
    *ptr3 = 123;
    assert(*ptr3 == 123);
    assert(*ptr1 == 99); // Asegurarse que ptr1 no cambió
    std::cout << "Ptr3 creado con ID: " << &ptr3 << ", Valor: " << *ptr3 << std::endl;

    std::cout << "Prueba básica de MPointer completada." << std::endl;
}

// --- Prueba de Lista Enlazada --- 
void test_linked_list() {
    std::cout << "\nEjecutando prueba de LinkedList con MPointers..." << std::endl;

    LinkedList<int> list;
    assert(list.isEmpty());
    assert(list.size() == 0);
    std::cout << "Lista inicialmente vacía." << std::endl;

    // Añadir elementos
    list.pushBack(10);
    list.pushBack(20);
    list.pushFront(5);
    // Lista esperada: 5 -> 10 -> 20
    assert(list.size() == 3);
    assert(!list.isEmpty());
    std::cout << "Elementos añadidos (5, 10, 20). Tamaño: " << list.size() << std::endl;
    list.print(); // Imprimir para verificación visual

    // Verificar valores
    assert(list.get(0) == 5);
    assert(list.get(1) == 10);
    assert(list.get(2) == 20);
    std::cout << "Valores verificados." << std::endl;

    // Eliminar del medio
    list.remove(1); // Eliminar el 10
    // Lista esperada: 5 -> 20
    assert(list.size() == 2);
    assert(list.get(0) == 5);
    assert(list.get(1) == 20);
    std::cout << "Elemento del medio (10) eliminado. Tamaño: " << list.size() << std::endl;
    list.print();

    // Eliminar del principio
    list.remove(0); // Eliminar el 5
    // Lista esperada: 20
    assert(list.size() == 1);
    assert(list.get(0) == 20);
    std::cout << "Elemento del principio (5) eliminado. Tamaño: " << list.size() << std::endl;
    list.print();

    // Añadir más elementos
    list.pushBack(30);
    list.pushFront(15);
    // Lista esperada: 15 -> 20 -> 30
    assert(list.size() == 3);
    assert(list.get(0) == 15);
    assert(list.get(1) == 20);
    assert(list.get(2) == 30);
    std::cout << "Elementos añadidos (15, 30). Tamaño: " << list.size() << std::endl;
    list.print();

    // Eliminar del final
    list.remove(2); // Eliminar el 30
    // Lista esperada: 15 -> 20
    assert(list.size() == 2);
    assert(list.get(0) == 15);
    assert(list.get(1) == 20);
    std::cout << "Elemento del final (30) eliminado. Tamaño: " << list.size() << std::endl;
    list.print();

    // Limpiar lista
    list.clear();
    assert(list.isEmpty());
    assert(list.size() == 0);
    std::cout << "Lista limpiada." << std::endl;
    list.print();

    std::cout << "Prueba de LinkedList completada." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);

    try {
        // MPointer<int>::Init(host, port); // Inicializar MPointer una vez
        MPointerConnection::Init(host, port); // Usar el Init centralizado
        test_basic_operations(); // Ejecutar prueba básica si se desea
        test_linked_list();    // Ejecutar prueba de lista enlazada
    } catch (const std::exception& e) {
        std::cerr << "Error durante las pruebas: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\nTodas las pruebas completadas exitosamente." << std::endl;
    return 0;
}