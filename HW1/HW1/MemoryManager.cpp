#include "MemoryManager.h"

#include <algorithm>
#include <iostream>


MemoryManager::MemoryManager(size_t maxMem, size_t frameSz)
    : maxMemorySize(maxMem),
    frameSize(frameSz),
    numFrames(maxMem / frameSz)
{
    memory.resize(numFrames);

    for (size_t i = 0; i < numFrames; ++i) {
        freeFrames.push_back(i);
    }
}

bool MemoryManager::allocateFrames(size_t numFrames, size_t processId, const std::vector<size_t>& pageSize) {

    return false;
}

void MemoryManager::deallocateFrames(size_t numFrames, size_t frameIndex, const std::vector<size_t>& pageSize) {

}
