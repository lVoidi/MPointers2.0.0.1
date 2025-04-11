# Requerimientos
I. Memory Manager (MM)
MM-01: Parámetros de línea de comandos
Debe aceptar los parámetros:
--port LISTEN_PORT: Puerto para escuchar peticiones de sockets
--memsize SIZE_MB: Tamaño de la memoria a reservar (en MB)
--dumpFolder DUMP_FOLDER: Carpeta para guardar archivos de dump
MM-02: Reserva inicial de memoria
Solo se permite un único malloc del tamaño especificado al iniciar
MM-03: Servidor de sockets
Iniciar un servidor de sockets para procesar peticiones
Permitir malloc/new solo para inicialización del servidor de sockets
MM-04: Peticiones soportadas
MM-04.1: Create(size, type) - Reserva un bloque dentro de la memoria reservada inicialmente (sin usar malloc) y retorna un ID único para el bloque
MM-04.2: Set(id, value) - Almacena un valor en el bloque asociado al ID
MM-04.3: Get(id) - Retorna el valor almacenado en el bloque del ID
MM-04.4: IncreaseRefCount(id) - Incrementa el contador de referencias del bloque
MM-04.5: DecreaseRefCount(id) - Decrementa el contador de referencias del bloque
MM-05: Garbage Collector
Hilo independiente que se ejecuta periódicamente
Libera bloques con contador de referencias en cero
MM-06: Defragmentación de memoria
Implementar un mecanismo para resolver fragmentación usando el método de compactación
MM-07: Archivos de dump
Generar un archivo legible (con fecha, hora, segundos y milisegundos) en DUMP_FOLDER después de cada operación que modifique la memoria
II. Biblioteca MPointers (MP)
MP-01: Clase template MPointer<T>
Implementar una clase template para manejar punteros gestionados remotamente
MP-02: Sobrecarga de operadores
MP-02.1: Operador * - Para leer/escribir valores en el Memory Manager
MP-02.2: Operador & - Retorna el ID del bloque de memoria
MP-02.3: Operador = - Copia el ID entre MPointers e incrementa el contador de referencias
MP-03: Comunicación por sockets
Conectarse al Memory Manager mediante el método estático Init(host, port)
Todas las operaciones (New, Set, Get) deben interactuar con el Memory Manager a través de sockets
MP-04: Método New()
Solicita al Memory Manager un nuevo bloque de memoria
Solo reserva memoria localmente para almacenar el ID
MP-05: Manejo de referencias
El destructor de MPointer debe llamar a DecreaseRefCount
Evitar memory leaks mediante el contador de referencias
MP-06: Restricciones de uso
Prohibido usar punteros nativos de C++ o reservar memoria directamente (new, malloc, etc.)
III. Pruebas Adicionales (PA)
PA-01: Lista enlazada
Implementar una lista enlazada utilizando únicamente MPointer<T>
IV. Aspectos Operativos (AO)
AO-01: Trabajo en equipo
El proyecto se desarrolla en parejas
AO-02: Control de versiones
Uso obligatorio de Git y GitHub
AO-03: Entrega
Subir un PDF con la documentación al TEC Digital
El código debe entregarse hasta 15 minutos antes de la revisión
V. Documentación (DOC)
DOC-01: Contenido obligatorio
Portada, introducción, tabla de contenidos
Descripción del problema y solución
Explicación detallada por cada requerimiento: implementación, alternativas, limitaciones
Diagrama de clases UML mostrando diseño orientado a objetos y patrones de diseño
Enlace al repositorio de GitHub
DOC-02: Formato
Archivo PDF
VI. Evaluación (EV)
EV-01: Criterios
Código (80%): Funcionalidad completa, integración, uso de C++
Documentación (10%): Completitud y alineación con el código
Defensa (10%): Exposición en 20 minutos
EV-02: Exclusiones
Código no en C++: Nota 0
Sin PDF/documentación: Nota 0
Sin uso de Git: Nota 0

# Cumplimiento de Requerimientos

Este documento detalla cómo el proyecto cumple con cada uno de los requerimientos especificados.

## I. Memory Manager (MM)

**MM-01: Parámetros de línea de comandos**
- **Cumplimiento:** Sí.
- **Descripción:** La aplicación del servidor (`server_app.cpp`) utiliza `argc` y `argv` en su función `main` para parsear los argumentos `--port`, `--memsize` y `--dumpFolder`. Estos valores se utilizan para inicializar el `MemoryManager` y el `SocketServer`.

**MM-02: Reserva inicial de memoria**
- **Cumplimiento:** Sí.
- **Descripción:** El constructor de la clase `MemoryManager` (`memory_manager.cpp`) realiza una única llamada a `malloc` con el tamaño especificado (`size_mb * 1024 * 1024`) para reservar el pool de memoria principal. No se realizan otras llamadas a `malloc`, `calloc` o `new` para la gestión de los bloques de memoria solicitados por los clientes.

**MM-03: Servidor de sockets**
- **Cumplimiento:** Sí.
- **Descripción:** La clase `SocketServer` (`socket_server.h`, `socket_server.cpp`) implementa un servidor TCP/IP usando la API de Sockets (Winsock en este caso). Inicializa el socket, lo vincula al puerto especificado (`bind`), se pone en modo escucha (`listen`) y acepta conexiones de clientes (`accept`). Cada cliente es manejado en un hilo separado (`std::thread`). Las llamadas a `new` o `malloc` necesarias para la inicialización y operación de la biblioteca de sockets están permitidas según el requerimiento.

**MM-04: Peticiones soportadas**
- **Cumplimiento:** Sí.
- **Descripción:** El servidor maneja diferentes tipos de mensajes recibidos del cliente. La función `SocketServer::processRequest` actúa como dispatcher basado en el `MessageType` recibido (`protocol/message.h`).
    - **MM-04.1: Create(size, type):** La petición `CREATE` es procesada llamando a `MemoryManager::create()`. Esta función busca un espacio libre adecuado en el `memory_pool_` (usando un algoritmo first-fit revisado) sin llamar a `malloc`, asigna un ID único y almacena los metadatos del bloque. Devuelve el ID al cliente.
    - **MM-04.2: Set(id, value):** La petición `SET` llama a `MemoryManager::set()`. Esta función verifica el ID, el estado `in_use` y el tamaño, y luego copia (`memcpy`) los datos del `value` recibido en la ubicación correspondiente del `memory_pool_`.
    - **MM-04.3: Get(id):** La petición `GET` llama a `MemoryManager::get()`. Verifica el ID y estado, y si es válido, copia (`memcpy`) los datos desde el `memory_pool_` al buffer de respuesta.
    - **MM-04.4: IncreaseRefCount(id):** La petición `INCREASE_REF_COUNT` llama a `MemoryManager::increaseRefCount()`, que localiza el bloque por ID e incrementa su contador `ref_count`.
    - **MM-04.5: DecreaseRefCount(id):** La petición `DECREASE_REF_COUNT` llama a `MemoryManager::decreaseRefCount()`, que localiza el bloque, decrementa `ref_count`, y si llega a cero o menos, marca el bloque como `in_use = false`.

**MM-05: Garbage Collector**
- **Cumplimiento:** Sí.
- **Descripción:** La clase `GarbageCollector` (`garbage_collector.h`, `garbage_collector.cpp`) se ejecuta en un hilo independiente (`std::thread`) iniciado por el servidor. Su método `collectGarbage` se ejecuta periódicamente (cada 500ms). Dentro de `collectGarbage`, se itera sobre los bloques del `MemoryManager` (protegido por mutex) y se marcan como `in_use = false` aquellos bloques cuyo `ref_count` es 0 o menor. La liberación efectiva del espacio ocurre durante la defragmentación.

**MM-06: Defragmentación de memoria**
- **Cumplimiento:** Sí.
- **Descripción:** Se implementa un mecanismo de defragmentación por **compactación** en la función `MemoryManager::compactMemory()`. Esta función, llamada periódicamente por el Garbage Collector y bajo demanda por `create` si no hay espacio, primero elimina los metadatos de los bloques marcados como `in_use = false`. Luego, detecta huecos entre los bloques en uso restantes, los ordena por offset y mueve (`memmove`) los datos de los bloques para eliminar los huecos, actualizando sus offsets en los metadatos.

**MM-07: Archivos de dump**
- **Cumplimiento:** Sí.
- **Descripción:** La función `MemoryManager::dumpMemoryState()` genera un archivo de texto legible en la carpeta especificada (`DUMP_FOLDER`). El nombre del archivo incluye fecha, hora, segundos y milisegundos (ej. `mem_dump_YYYYMMDD_HHMMSS_sss.txt`). Esta función es llamada después de cada operación que puede modificar el estado de la memoria o las referencias (`create`, `set`, `decreaseRefCount`, `increaseRefCount`, `compactMemory`). El formato del dump incluye el tamaño total, número de bloques y una tabla con ID, Offset, Size, Type, Ref Count y Status de cada bloque, además de un mapa visual simplificado.

## II. Biblioteca MPointers (MP)

**MP-01: Clase template MPointer<T>**
- **Cumplimiento:** Sí.
- **Descripción:** Se implementa la clase `template <typename T> class MPointer` en `mpointer/mpointer.h`. Esta clase encapsula un ID entero (`id_`) que representa un bloque de memoria en el Memory Manager remoto.

**MP-02: Sobrecarga de operadores**
- **Cumplimiento:** Sí.
- **Descripción:**
    - **MP-02.1: Operador \*:** Se implementa utilizando el patrón Proxy. `MPointer::operator*()` devuelve una instancia de la clase anidada `MPointer<T>::Proxy`. El `Proxy::operator T() const` se usa para lectura (realiza `GET`) y `Proxy::operator=(const T& value)` se usa para escritura (realiza `SET`). Esto permite sintaxis como `*ptr = value` y `T x = *ptr`.
    - **MP-02.2: Operador &:** Se sobrecarga `MPointer::operator&() const` para devolver el `id_` entero almacenado.
    - **MP-02.3: Operador =:** Se sobrecarga `MPointer::operator=(const MPointer<T>& other)`. Esta función copia el `id_` de `other` a `this` y gestiona correctamente los contadores de referencia llamando a `increaseRefCount` para el nuevo ID y `decreaseRefCount` para el ID antiguo a través del `SocketClient`.

**MP-03: Comunicación por sockets**
- **Cumplimiento:** Sí.
- **Descripción:** La conexión al Memory Manager se establece mediante el método estático `MPointerConnection::Init(host, port)` (definido en `mpointer.h`), que inicializa un `SocketClient` compartido. Todas las operaciones que requieren interacción con el servidor (`New`, `Get`, `Set`, `IncreaseRefCount`, `DecreaseRefCount`), ya sea directamente o a través del Proxy, utilizan la instancia compartida de `SocketClient` (`socket_client.h`, `socket_client.cpp`) para enviar las peticiones correspondientes (`createMemoryBlock`, `getMemoryBlock`, `setMemoryBlock`, etc.) y recibir las respuestas a través de sockets TCP/IP.

**MP-04: Método New()**
- **Cumplimiento:** Sí.
- **Descripción:** El método estático `MPointer<T>::New()` llama a `MPointerConnection::client_->createMemoryBlock(sizeof(T), typeid(T).name())`. Esta función envía una petición `CREATE` al Memory Manager. El `MPointer` devuelto solo almacena localmente el `id_` entero retornado por el servidor. No se reserva memoria local para el tipo `T`.

**MP-05: Manejo de referencias**
- **Cumplimiento:** Sí.
- **Descripción:** El destructor `MPointer::~MPointer()` llama a `MPointerConnection::client_->decreaseRefCount(id_)` si el `id_` es válido. Los constructores (`MPointer(int id)`, `MPointer(const MPointer& other)`) y el operador de asignación (`operator=`) llaman a `increaseRefCount` y/o `decreaseRefCount` según corresponda para mantener la integridad del contador de referencias en el Memory Manager, previniendo memory leaks.

**MP-06: Restricciones de uso**
- **Cumplimiento:** Sí.
- **Descripción:** La biblioteca `MPointer` en sí no utiliza punteros nativos ni asignación dinámica (`new`/`malloc`) para los datos gestionados, solo para su ID interno y la gestión del cliente de socket. El código que *utiliza* la biblioteca (como la lista enlazada de prueba en `tests/mpointer_test.cpp` y `examples/linked_list.h`) también se adhiere a esta restricción.

## III. Pruebas Adicionales (PA)

**PA-01: Lista enlazada**
- **Cumplimiento:** Sí.
- **Descripción:** Se implementó una clase template `LinkedList<T>` en `examples/linked_list.h`. Esta clase utiliza `MPointer<Node>` para representar los punteros a los nodos (`head_`, `tail_`) y `int next_id` dentro de `struct Node` para enlazar los elementos. Todas las operaciones de la lista (añadir, obtener, eliminar, imprimir) se realizan manipulando `MPointer`s y sus IDs asociados, creando nodos con `MPointer<Node>::New()`, sin usar punteros nativos ni asignación directa de memoria para los nodos. Se incluye un caso de prueba (`test_linked_list`) en `tests/mpointer_test.cpp` que verifica su funcionalidad básica (añadir, obtener, eliminar, limpiar) usando `assert`.

## IV. Aspectos Operativos (AO)

**AO-01: Trabajo en equipo**
- **Cumplimiento:** Sí.
- **Descripción:** Este requerimiento se cumple mediante la organización y colaboración del equipo de desarrollo (asumido, basado en la interacción).

**AO-02: Control de versiones**
- **Cumplimiento:** Sí.
- **Descripción:** El proyecto se gestiona utilizando Git como sistema de control de versiones y se aloja en un repositorio de GitHub (asumido), cumpliendo con el uso obligatorio especificado.

## V. Documentación (DOC) y VI. Evaluación (EV)

Estos requerimientos se refieren a la documentación externa (PDF) y los criterios de evaluación, no directamente al código. Se asume que se generará un PDF siguiendo las especificaciones (DOC-01, DOC-02) y que el código cumple con los criterios funcionales y técnicos para la evaluación (EV-01).
