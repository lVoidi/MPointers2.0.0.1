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
    // Acquire lock to safely access memory manager data
    bool block_freed = false;
    { // Scope for the lock guard
        std::lock_guard<std::recursive_mutex> lock(memory_manager_->memory_mutex_);
        // Iterate through all blocks managed by the MemoryManager
        for (auto it = memory_manager_->blocks_.begin(); it != memory_manager_->blocks_.end(); ++it) {
            // Check if the block is currently marked as in use but has a reference count of zero or less
            if (it->second.in_use && it->second.ref_count <= 0) {
                // Mark the block as no longer in use. It will be cleaned up later by compactMemory.
                std::cout << "GC: Marking block ID " << it->first << " as free (ref count " << it->second.ref_count << ")." << std::endl;
                it->second.in_use = false;
                block_freed = true;
            }
        }
        // Optional: Trigger a memory dump immediately after freeing blocks if needed for debugging
        // if (block_freed) {
        //     memory_manager_->dumpMemoryState();
        // }
    } // Lock guard goes out of scope, mutex is released


    // --- Defragmentation Logic ---
    // Keep track of runs to periodically trigger defragmentation
    static int run_count = 0;
    run_count++;

    // Trigger defragmentation (compaction) every N runs (e.g., every 10 runs = 5 seconds)
    // The compactMemory function internally handles acquiring the necessary mutex.
    if (run_count % 10 == 0) {
        std::cout << "Garbage collector initiating periodic defragmentation..." << std::endl;
        memory_manager_->compactMemory();
    }
}