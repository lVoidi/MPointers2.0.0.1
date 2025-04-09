//
// Created by roarb on 08/04/2025.
//

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#endif //MEMORY_MANAGER_H

class MemoryManager {
public:
    MemoryManager(size_t size_mb, const std::string& dump_folder);
    ~MemoryManager();

    int create(size_t size, const std::string& type);
    bool set(int id, const void* value, size_t size);
    bool get(int id, void* result, size_t size);
    bool increaseRefCount(int id);
    bool decreaseRefCount(int id);

    void startGarbageCollector();
    void dumpMemoryState();

private:
    char* memory_pool_;             // Single memory allocation
    size_t memory_size_;
    std::string dump_folder_;

    struct MemoryBlock {
        size_t offset;
        size_t size;
        std::string type;
        int ref_count;
        bool in_use;
    };

    std::unordered_map<int, MemoryBlock> blocks_;
    std::mutex memory_mutex_;
    std::thread gc_thread_;

    void compactMemory();           // Memory defragmentation
};