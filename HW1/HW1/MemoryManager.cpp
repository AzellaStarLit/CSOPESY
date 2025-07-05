#include "MemoryManager.h"

#include <algorithm>
#include <iostream>


MemoryManager::MemoryManager() {
    // Initialize memory frames
    memory.resize(numFrames);

    // Initialize all frames as free
    for (size_t i = 0; i < numFrames; ++i) {
        freeFrames.push_back(i);
    }
}

bool MemoryManager::allocateFrames(size_t numFrames, size_t processId, const std::vector<size_t>& pageSize) {

    return false;
}

void MemoryManager::deallocateFrames(size_t numFrames, size_t frameIndex, const std::vector<size_t>& pageSize) {

}
