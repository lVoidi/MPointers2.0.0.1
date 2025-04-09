//
// Created by roarb on 08/04/2025.
//

// garbage_collector.cpp
#include "garbage_collector.h"
#include "memory_manager.h"
#include <iostream>

GarbageCollector::GarbageCollector(MemoryManager* memory_manager)
    : memory_manager_(memory_manager), running_(false) {
}

GarbageCollector::~GarbageCollector() {
    stop();
}

void GarbageCollector::start() {
    running_ = true;
    collector_thread_ = std::thread([this]() {
        while (running_) {
            collectGarbage();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
}

void GarbageCollector::stop() {
    if (running_) {
        running_ = false;
        if (collector_thread_.joinable()) {
            collector_thread_.join();
        }
    }
}

void GarbageCollector::collectGarbage() {
    // The actual implementation will call memory_manager's methods
    // to clean up blocks with ref_count == 0 and periodically defragment memory

    // For now, just log the activity
    static int run_count = 0;
    run_count++;

    // Every 10 runs, initiate defragmentation
    if (run_count % 10 == 0) {
        std::cout << "Garbage collector running defragmentation..." << std::endl;
        memory_manager_->compactMemory();
    }
}