//
// Created by roarb on 08/04/2025.
//
// memory_manager.cpp
#include "memory_manager.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cstring>

MemoryManager::MemoryManager(size_t size_mb, const std::string& dump_folder)
    : memory_size_(size_mb * 1024 * 1024), dump_folder_(dump_folder) {
    // Allocate memory pool with a single malloc call
    memory_pool_ = static_cast<char*>(malloc(memory_size_));
    if (!memory_pool_) {
        throw std::runtime_error("Failed to allocate memory pool");
    }

    // Initialize memory to zero
    memset(memory_pool_, 0, memory_size_);

    std::cout << "Memory manager initialized with " << size_mb << "MB" << std::endl;
}

MemoryManager::~MemoryManager() {
    // Free the entire memory pool
    free(memory_pool_);
    memory_pool_ = nullptr;
}

int MemoryManager::create(size_t size, const std::string& type) {
    std::lock_guard<std::mutex> lock(memory_mutex_);

    // Find first suitable free block
    size_t offset = 0;
    size_t available_size = 0;
    bool found = false;

    // Simple first-fit algorithm
    for (size_t i = 0; i < memory_size_; i += available_size) {
        available_size = 0;

        // Check each byte until we find a sequence long enough
        bool in_use = false;
        for (const auto& [id, block] : blocks_) {
            if (block.in_use && block.offset <= i && i < block.offset + block.size) {
                in_use = true;
                // Skip to end of this block
                i = block.offset + block.size - 1; // -1 because loop will add available_size (which is 0)
                break;
            }
        }

        if (!in_use) {
            // Count consecutive free bytes
            available_size = 1;
            for (size_t j = i + 1; j < memory_size_ && available_size < size; j++) {
                bool byte_in_use = false;
                for (const auto& [id, block] : blocks_) {
                    if (block.in_use && block.offset <= j && j < block.offset + block.size) {
                        byte_in_use = true;
                        break;
                    }
                }
                if (byte_in_use) break;
                available_size++;
            }

            if (available_size >= size) {
                offset = i;
                found = true;
                break;
            }
        }
    }

    if (!found) {
        // Try to defragment and retry
        compactMemory();

        // Simple allocation after defragmentation - just get the first free block
        offset = 0;
        for (const auto& [id, block] : blocks_) {
            if (block.in_use) {
                size_t block_end = block.offset + block.size;
                if (block_end > offset) {
                    offset = block_end;
                }
            }
        }

        // Check if we have enough space after all blocks
        if (offset + size > memory_size_) {
            return -1;  // Out of memory
        }
    }

    // Create new memory block
    static int next_id = 1;
    int id = next_id++;

    blocks_[id] = {
        .offset = offset,
        .size = size,
        .type = type,
        .ref_count = 1,
        .in_use = true
    };

    dumpMemoryState();
    return id;
}

bool MemoryManager::set(int id, const void* value, size_t size) {
    std::lock_guard<std::mutex> lock(memory_mutex_);

    auto it = blocks_.find(id);
    if (it == blocks_.end() || !it->second.in_use) {
        return false;  // Invalid ID or block not in use
    }

    MemoryBlock& block = it->second;
    if (size > block.size) {
        return false;  // Value too large for block
    }

    // Copy value to memory pool
    memcpy(memory_pool_ + block.offset, value, size);

    dumpMemoryState();
    return true;
}

bool MemoryManager::get(int id, void* result, size_t size) {
    std::lock_guard<std::mutex> lock(memory_mutex_);

    auto it = blocks_.find(id);
    if (it == blocks_.end() || !it->second.in_use) {
        return false;  // Invalid ID or block not in use
    }

    MemoryBlock& block = it->second;
    if (size > block.size) {
        return false;  // Requested size too large
    }

    // Copy from memory pool to result buffer
    memcpy(result, memory_pool_ + block.offset, size);
    return true;
}

bool MemoryManager::increaseRefCount(int id) {
    std::lock_guard<std::mutex> lock(memory_mutex_);

    auto it = blocks_.find(id);
    if (it == blocks_.end() || !it->second.in_use) {
        return false;  // Invalid ID or block not in use
    }

    it->second.ref_count++;
    return true;
}

bool MemoryManager::decreaseRefCount(int id) {
    std::lock_guard<std::mutex> lock(memory_mutex_);

    auto it = blocks_.find(id);
    if (it == blocks_.end() || !it->second.in_use) {
        return false;  // Invalid ID or block not in use
    }

    it->second.ref_count--;

    // If reference count is 0, mark block as unused (will be cleaned up by GC)
    if (it->second.ref_count <= 0) {
        it->second.in_use = false;
    }

    dumpMemoryState();
    return true;
}

void MemoryManager::compactMemory() {
    std::cout << "Starting memory defragmentation..." << std::endl;

    // First, remove all blocks with in_use=false
    for (auto it = blocks_.begin(); it != blocks_.end();) {
        if (!it->second.in_use) {
            it = blocks_.erase(it);
        } else {
            ++it;
        }
    }

    // Sort blocks by offset
    std::vector<std::pair<int, MemoryBlock>> sorted_blocks;
    for (const auto& [id, block] : blocks_) {
        sorted_blocks.push_back({id, block});
    }

    std::sort(sorted_blocks.begin(), sorted_blocks.end(),
              [](const auto& a, const auto& b) {
                  return a.second.offset < b.second.offset;
              });

    // Compact blocks
    size_t current_offset = 0;
    for (auto& [id, block] : sorted_blocks) {
        if (block.offset > current_offset) {
            // Move block data to new offset
            memmove(memory_pool_ + current_offset,
                    memory_pool_ + block.offset, block.size);

            // Update block offset
            block.offset = current_offset;
            blocks_[id].offset = current_offset;
        }
        current_offset += block.size;
    }

    std::cout << "Memory defragmentation complete" << std::endl;
    dumpMemoryState();
}

void MemoryManager::dumpMemoryState() {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch()) % 1000;

    std::stringstream filename;
    filename << dump_folder_ << "/mem_dump_";
    filename << std::put_time(std::localtime(&now_time), "%Y%m%d_%H%M%S");
    filename << "_" << std::setfill('0') << std::setw(3) << now_ms.count() << ".txt";

    std::ofstream dump_file(filename.str());
    if (!dump_file) {
        std::cerr << "Failed to create memory dump file: " << filename.str() << std::endl;
        return;
    }

    dump_file << "Memory Dump - "
              << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S")
              << "." << std::setfill('0') << std::setw(3) << now_ms.count() << "\n\n";

    dump_file << "Total Memory: " << memory_size_ << " bytes\n";
    dump_file << "Block Count: " << blocks_.size() << "\n\n";

    dump_file << "Blocks:\n";
    dump_file << "-------------------------------------------------------------------------\n";
    dump_file << "ID\tOffset\tSize\tType\tRef Count\tStatus\n";
    dump_file << "-------------------------------------------------------------------------\n";

    for (const auto& [id, block] : blocks_) {
        dump_file << id << "\t"
                  << block.offset << "\t"
                  << block.size << "\t"
                  << block.type << "\t"
                  << block.ref_count << "\t\t"
                  << (block.in_use ? "In Use" : "Free") << "\n";
    }

    dump_file << "-------------------------------------------------------------------------\n\n";

    dump_file << "Memory Map (showing first 100 bytes or until end):\n";
    dump_file << "-------------------------------------------------------------------------\n";

    size_t display_size = std::min(memory_size_, static_cast<size_t>(100));
    for (size_t i = 0; i < display_size; i++) {
        bool in_block = false;
        for (const auto& [id, block] : blocks_) {
            if (block.in_use && block.offset <= i && i < block.offset + block.size) {
                dump_file << "[" << id << "]";
                in_block = true;
                break;
            }
        }

        if (!in_block) {
            dump_file << "[ ]";
        }

        if ((i + 1) % 10 == 0) {
            dump_file << "\n";
        }
    }

    dump_file.close();
    std::cout << "Memory dump created: " << filename.str() << std::endl;
}

void MemoryManager::startGarbageCollector() {
    // This will be implemented separately in the GarbageCollector class
    std::cout << "Garbage collector started" << std::endl;
}