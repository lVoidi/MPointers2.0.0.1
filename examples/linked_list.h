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
        int next_id;

        // Constructor con valor
        Node(const T& value) : data(value), next_id(-1) {}
        
        // Constructor por defecto
        Node() : data(), next_id(-1) {}
    };

    MPointer<Node> head_;
    MPointer<Node> tail_;
    size_t size_;
};

// Implementación de los métodos de LinkedList

template <typename T>
void LinkedList<T>::pushBack(const T& value) {
    MPointer<Node> newNode = MPointer<Node>::New();
    Node nodeData(value);
    *newNode = nodeData;

    if (isEmpty()) {
        head_ = newNode;
        tail_ = newNode;
    } else {
        Node tailNode = *tail_;
        tailNode.next_id = &newNode;
        *tail_ = tailNode;
        tail_ = newNode;
    }

    size_++;
    // Liberar newNode para que su destructor no haga decref
    newNode.release();
}

template <typename T>
void LinkedList<T>::pushFront(const T& value) {
    MPointer<Node> newNode = MPointer<Node>::New();
    Node nodeData(value);
    if (head_.isValid()) {
        nodeData.next_id = &head_;
    }
    *newNode = nodeData;
    
    head_ = newNode;
    if (isEmpty()) {
        tail_ = newNode;
    }
    
    size_++;
    // Liberar newNode para que su destructor no haga decref
    newNode.release();
}

template <typename T>
T LinkedList<T>::get(size_t index) const {
    if (index >= size_) {
        throw std::out_of_range("Índice fuera de rango");
    }

    MPointer<Node> current = head_;
    for (size_t i = 0; i < index; i++) {
        Node currentNode = *current;
        if (currentNode.next_id < 0) {
             throw std::runtime_error("Error de lógica: Se alcanzó el final de la lista inesperadamente en get");
        }
        current = MPointer<Node>(currentNode.next_id);
    }

    Node resultNode = *current;
    current.release();
    return resultNode.data;
}

template <typename T>
void LinkedList<T>::clear() {
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
        Node headNode = *head_;
        if(headNode.next_id >= 0) {
            head_ = MPointer<Node>(headNode.next_id);
        } else {
            head_ = MPointer<Node>();
            tail_ = MPointer<Node>();
        }
    } else {
        MPointer<Node> prev = head_;
        for (size_t i = 0; i < index - 1; i++) {
            Node prevNode = *prev;
            if (prevNode.next_id < 0) {
                 throw std::runtime_error("Error de lógica: Se alcanzó el final de la lista inesperadamente en remove");
            }
            prev = MPointer<Node>(prevNode.next_id);
        }

        Node prevNode = *prev;
        int remove_id = prevNode.next_id;
        if(remove_id < 0) {
             throw std::runtime_error("Error de lógica: El nodo a eliminar no existe");
        }
        MPointer<Node> toRemove = MPointer<Node>(remove_id);
        Node toRemoveNode = *toRemove;

        prevNode.next_id = toRemoveNode.next_id;
        *prev = prevNode;

        if (toRemoveNode.next_id < 0) {
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
        Node currentNode = *current;
        std::cout << currentNode.data << " ";
        int next_id = currentNode.next_id;
        if (next_id >= 0) {
            current = MPointer<Node>(next_id);
        } else {
            current.release();
            break;
        }
    }
    std::cout << std::endl;
}

#endif //LINKED_LIST_H
