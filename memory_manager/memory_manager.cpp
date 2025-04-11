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
#include <algorithm> // Para std::sort

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
    std::lock_guard<std::recursive_mutex> lock(memory_mutex_);
    std::cout << "[MemoryManager] Attempting to create block of size " << size << std::endl;

    size_t offset = 0;
    bool found = false;

    // --- Revised First-Fit Algorithm ---
    std::cout << "[MemoryManager] Starting revised first-fit search..." << std::endl;
    size_t current_offset = 0;
    while (current_offset < memory_size_) {
        // Check if current_offset is within an existing used block
        bool occupied = false;
        size_t occupying_block_end = current_offset;
        int occupying_block_id = -1;
        for (const auto& [id, block] : blocks_) {
            if (block.in_use && block.offset <= current_offset && current_offset < block.offset + block.size) {
                occupied = true;
                occupying_block_end = block.offset + block.size;
                occupying_block_id = id;
                break;
            }
        }

        if (occupied) {
            // If occupied, skip past the occupying block
            std::cout << "[MemoryManager] Offset " << current_offset << " is occupied by block " << occupying_block_id << ". Skipping to " << occupying_block_end << std::endl;
            current_offset = occupying_block_end;
            continue; // Check the next position immediately after the block
        }

        // If not occupied, find the size of the contiguous free block starting at current_offset
        size_t available_size = 0;
        size_t check_offset = current_offset;
        while (check_offset < memory_size_) {
            bool next_byte_occupied = false;
            for (const auto& [id, block] : blocks_) {
                if (block.in_use && block.offset <= check_offset && check_offset < block.offset + block.size) {
                    next_byte_occupied = true;
                    break;
                }
            }
            if (next_byte_occupied) {
                break; // End of the contiguous free block
            }
            available_size++;
            check_offset++;
        }
        std::cout << "[MemoryManager] Found free block at offset " << current_offset << " of size " << available_size << std::endl;

        // Check if this free block is large enough
        if (available_size >= size) {
            offset = current_offset;
            found = true;
            std::cout << "[MemoryManager] Found suitable block at offset " << offset << std::endl;
            break; // Exit the while loop, suitable block found
        } else {
            // Free block not large enough, move to the position after this free block
            current_offset += available_size;
            // If available_size was 0 (e.g., we hit the end of memory immediately), ensure we advance.
            if (available_size == 0) {
                current_offset++;
            }
        }
    }
    std::cout << "[MemoryManager] Finished first-fit search. Found: " << (found ? "Yes" : "No") << std::endl;
    // --- End of Revised First-Fit Algorithm ---

    if (!found) {
        std::cout << "[MemoryManager] No suitable block found initially, attempting defragmentation..." << std::endl; // Log
        // Try to defragment and retry
        compactMemory();

        // Simple allocation after defragmentation - find end of last block
        std::cout << "[MemoryManager] Defragmentation complete, retrying allocation..." << std::endl; // Log
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
            std::cerr << "[MemoryManager] Out of memory even after defragmentation." << std::endl; // Log
            return -1;  // Out of memory
        }
        std::cout << "[MemoryManager] Found space after defragmentation at offset " << offset << std::endl; // Log
        found = true; // Mark as found since we allocated after defrag
    }

    // If we still haven't found space (e.g., defrag didn't help enough or wasn't needed but space check failed)
    if (!found) {
         std::cerr << "[MemoryManager] Failed to find or create space for the requested size." << std::endl;
         return -1; // Should not happen if logic above is correct, but safeguard
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
    std::cout << "[MemoryManager] Created block ID " << id << " at offset " << offset << " size " << size << std::endl; // Add log

    dumpMemoryState();
    return id;
}

bool MemoryManager::set(int id, const void* value, size_t size) {
    std::lock_guard<std::recursive_mutex> lock(memory_mutex_);

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
    std::lock_guard<std::recursive_mutex> lock(memory_mutex_);

    auto it = blocks_.find(id);
    if (it == blocks_.end()) {
        std::cerr << "[MemoryManager] GET failed for ID " << id << ": Block not found in map." << std::endl;
        return false; // ID no existe
    }
    
    if (!it->second.in_use) {
         std::cerr << "[MemoryManager] GET failed for ID " << id << ": Block marked as not in use." << std::endl;
         return false; // Bloque no en uso
    }

    MemoryBlock& block = it->second;
    if (size > block.size) {
        std::cerr << "[MemoryManager] GET failed for ID " << id << ": Requested size (" << size << ") > block size (" << block.size << ")." << std::endl;
        return false;  // Requested size too large
    }

    // Copy from memory pool to result buffer
    std::cout << "[MemoryManager] GET successful for ID " << id << ". Copying " << size << " bytes." << std::endl; // Log éxito
    memcpy(result, memory_pool_ + block.offset, size);
    return true;
}

bool MemoryManager::increaseRefCount(int id) {
    std::lock_guard<std::recursive_mutex> lock(memory_mutex_);

    auto it = blocks_.find(id);
    if (it == blocks_.end() || !it->second.in_use) {
        return false;  // Invalid ID or block not in use
    }

    it->second.ref_count++;
    return true;
}

bool MemoryManager::decreaseRefCount(int id) {
    std::lock_guard<std::recursive_mutex> lock(memory_mutex_);

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
    std::lock_guard<std::recursive_mutex> lock(memory_mutex_);
    std::cout << "Starting memory defragmentation..." << std::endl;

    // Verificar si hay bloques que necesitan liberarse
    bool freeBlocksExist = false;
    for (const auto& [id, block] : blocks_) {
        if (!block.in_use) {
            freeBlocksExist = true;
            break;
        }
    }

    if (!freeBlocksExist) {
        std::cout << "No free blocks to remove, checking for fragmentation..." << std::endl;
    } else {
        std::cout << "Removing free blocks..." << std::endl;
        // First, remove all blocks with in_use=false
        for (auto it = blocks_.begin(); it != blocks_.end();) {
            if (!it->second.in_use) {
                std::cout << "Removing block with ID: " << it->first << std::endl;
                it = blocks_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Verificar si hay fragmentación
    bool fragmented = false;
    size_t last_block_end = 0;
    
    // Sort blocks by offset
    std::vector<std::pair<int, MemoryBlock>> sorted_blocks;
    for (const auto& [id, block] : blocks_) {
        sorted_blocks.push_back({id, block});
    }

    std::sort(sorted_blocks.begin(), sorted_blocks.end(),
              [](const auto& a, const auto& b) {
                  return a.second.offset < b.second.offset;
              });
    
    // Verificar si hay espacios entre bloques
    for (size_t i = 0; i < sorted_blocks.size(); i++) {
        const auto& [id, block] = sorted_blocks[i];
        if (block.offset > last_block_end) {
            std::cout << "Found gap between blocks: " 
                      << last_block_end << " to " << block.offset 
                      << " (size: " << block.offset - last_block_end << " bytes)" << std::endl;
            fragmented = true;
        }
        last_block_end = block.offset + block.size;
    }
    
    if (!fragmented) {
        std::cout << "Memory is not fragmented, no compaction needed" << std::endl;
        return;
    }

    std::cout << "Compacting memory blocks..." << std::endl;

    // Compact blocks
    size_t current_offset = 0;
    for (auto& [id, block] : sorted_blocks) {
        if (block.offset > current_offset) {
            std::cout << "Moving block ID " << id 
                      << " from offset " << block.offset 
                      << " to " << current_offset 
                      << " (size: " << block.size << " bytes)" << std::endl;
            
            // Move block data to new offset
            memmove(memory_pool_ + current_offset,
                    memory_pool_ + block.offset, block.size);

            // Update block offset
            block.offset = current_offset;
            blocks_[id].offset = current_offset;
        }
        current_offset += block.size;
    }

    // Calcular memoria liberada
    size_t free_memory = memory_size_ - current_offset;
    double free_percentage = (static_cast<double>(free_memory) / memory_size_) * 100.0;
    
    std::cout << "Memory defragmentation complete" << std::endl;
    std::cout << "Total memory: " << memory_size_ << " bytes" << std::endl;
    std::cout << "Used memory: " << current_offset << " bytes ("
              << (100.0 - free_percentage) << "%)" << std::endl;
    std::cout << "Free memory: " << free_memory << " bytes ("
              << free_percentage << "%)" << std::endl;
              
    dumpMemoryState();
}

void MemoryManager::dumpMemoryState() {
    std::cout << "[Dump] Starting dumpMemoryState..." << std::endl; // Log inicio dump
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
    std::cout << "[Dump] Memory dump created: " << filename.str() << std::endl;
    std::cout << "[Dump] Finished dumpMemoryState." << std::endl; // Log fin dump
}

void MemoryManager::startGarbageCollector() {
    // This will be implemented separately in the GarbageCollector class
    std::cout << "Garbage collector started" << std::endl;
}