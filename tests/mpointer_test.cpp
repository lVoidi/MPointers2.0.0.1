//
// Created by roarb on 09/04/2025.
//
//
// Created by roarb on 08/04/2025.
//

#include <iostream>
#include "../mpointer/mpointer.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);

    try {
        std::cout << "Iniciando prueba..." << std::endl << std::flush;

        // Inicializar la conexión con el servidor
        MPointer<int>::Init(host, port);
        std::cout << "Conexión establecida con el servidor" << std::endl << std::flush;

        // Crear un nuevo MPointer
        MPointer<int> intPtr = MPointer<int>::New();
        std::cout << "Creado MPointer con ID: " << &intPtr << std::endl << std::flush;

        // Asignar valor
        *intPtr = 42;
        std::cout << "Asignado valor 42" << std::endl << std::flush;

        // Leer valor
        int value = *intPtr;
        std::cout << "Valor leído: " << value << std::endl << std::flush;

        // Probar copia de punteros
        MPointer<int> intPtr2;
        intPtr2 = intPtr;
        std::cout << "Copiado MPointer, ID: " << &intPtr2 << std::endl << std::flush;

        // Modificar a través del segundo puntero
        *intPtr2 = 100;
        std::cout << "Modificado valor a 100 a través del segundo puntero" << std::endl << std::flush;

        // Verificar que el cambio se refleja en el primer puntero
        value = *intPtr;
        std::cout << "Valor leído a través del primer puntero: " << value << std::endl << std::flush;

        std::cout << "Prueba completada con éxito" << std::endl << std::flush;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl << std::flush;
        return 1;
    }

    return 0;
}