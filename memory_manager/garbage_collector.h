//
// Created by roarb on 08/04/2025.
//

#ifndef GARBAGE_COLLECTOR_H
#define GARBAGE_COLLECTOR_H

#endif //GARBAGE_COLLECTOR_H
#pragma once
#include <thread>
#include <atomic>
#include <chrono>

class MemoryManager;

class GarbageCollector {
public:
    GarbageCollector(MemoryManager* memory_manager);
    ~GarbageCollector();

    void start();
    void stop();

private:
    MemoryManager* memory_manager_;
    std::atomic<bool> running_;
    std::thread collector_thread_;

    void collectGarbage();
};