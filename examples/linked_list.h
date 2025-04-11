//
// Created by roarb on 08/04/2025.
//

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "../mpointer/mpointer.h"
#include <iostream>
#include <stdexcept>

template <typename T>
class LinkedList {
public:
    // Constructor
    LinkedList() : head_(), tail_(), size_(0) {}

    // Destructor
    ~LinkedList() {
        clear();
    }

    // Obtener tamaño de la lista
    size_t size() const {
        return size_;
    }

    // Verificar si la lista está vacía
    bool isEmpty() const {
        return size_ == 0;
    }

    // Añadir elemento al final de la lista
    void pushBack(const T& value);

    // Añadir elemento al inicio de la lista
    void pushFront(const T& value);

    // Obtener elemento en una posición específica
    T get(size_t index) const;

    // Eliminar todos los elementos
    void clear();

    // Eliminar elemento en una posición específica
    void remove(size_t index);

    // Imprimir todos los elementos (para depuración)
    void print() const;

private:
    struct Node {
        T data;
        MPointer<Node> next;

        // Constructor con valor
        Node(const T& value) : data(value), next() {}
        
        // Constructor por defecto
        Node() : data(), next() {}
    };

    MPointer<Node> head_;
    MPointer<Node> tail_;
    size_t size_;
};

// Implementación de los métodos de LinkedList

template <typename T>
void LinkedList<T>::pushBack(const T& value) {
    MPointer<Node> newNode = MPointer<Node>::New();
    *newNode = Node(value);

    if (isEmpty()) {
        head_ = newNode;
        tail_ = newNode;
    } else {
        (*tail_).next = newNode;
        tail_ = newNode;
    }

    size_++;
}

template <typename T>
void LinkedList<T>::pushFront(const T& value) {
    MPointer<Node> newNode = MPointer<Node>::New();
    *newNode = Node(value);

    if (isEmpty()) {
        head_ = newNode;
        tail_ = newNode;
    } else {
        (*newNode).next = head_;
        head_ = newNode;
    }

    size_++;
}

template <typename T>
T LinkedList<T>::get(size_t index) const {
    if (index >= size_) {
        throw std::out_of_range("Índice fuera de rango");
    }

    MPointer<Node> current = head_;
    for (size_t i = 0; i < index; i++) {
        current = (*current).next;
    }

    return (*current).data;
}

template <typename T>
void LinkedList<T>::clear() {
    // Los MPointers se encargarán de decrementar las referencias
    // cuando se asigna nullptr
    head_ = MPointer<Node>();
    tail_ = MPointer<Node>();
    size_ = 0;
}

template <typename T>
void LinkedList<T>::remove(size_t index) {
    if (index >= size_) {
        throw std::out_of_range("Índice fuera de rango");
    }

    if (index == 0) {
        // Eliminar el primer elemento
        MPointer<Node> temp = head_;
        head_ = (*head_).next;
        
        // Si la lista queda vacía, actualizar tail_
        if (head_.getId() < 0) {
            tail_ = MPointer<Node>();
        }
    } else {
        // Encontrar el nodo anterior al que se eliminará
        MPointer<Node> prev = head_;
        for (size_t i = 0; i < index - 1; i++) {
            prev = (*prev).next;
        }

        // Guardar referencia al nodo a eliminar
        MPointer<Node> toRemove = (*prev).next;

        // Actualizar next del nodo anterior
        (*prev).next = (*toRemove).next;

        // Si eliminamos el último nodo, actualizar tail_
        if (index == size_ - 1) {
            tail_ = prev;
        }
    }

    size_--;
}

template <typename T>
void LinkedList<T>::print() const {
    if (isEmpty()) {
        std::cout << "Lista vacía" << std::endl;
        return;
    }

    MPointer<Node> current = head_;
    std::cout << "Lista: ";
    while (current.isValid()) {
        std::cout << (*current).data << " ";
        current = (*current).next;
    }
    std::cout << std::endl;
}

#endif //LINKED_LIST_H
