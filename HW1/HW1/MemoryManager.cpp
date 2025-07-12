#include "MemoryManager.h"

#include <algorithm>
#include <iostream>


MemoryManager::MemoryManager(size_t maxMem, size_t frameSz)
    : maxMemorySize(maxMem),
    frameSize(frameSz),
    numFrames((frameSz == 0) ? 0 : maxMem / frameSz)
{
    memory.resize(numFrames);

    for (size_t i = 0; i < numFrames; ++i) {
        freeFrames.push_back(i);
    }
}

bool MemoryManager::allocateFrames(size_t numFrames, size_t processId, const std::vector<size_t>&) {

    if (freeFrames.size() > numFrames) return false;

    size_t consecutive = 0; //consecutive free frames
    size_t startIndex = 0; //start of free frames

    for (size_t i = 0; i < memory.size(); i++) {

        if (!memory[i].occupied) {
            if (consecutive == 0) startIndex = i;
            ++consecutive;

            //when you have enough frames
            if (consecutive == numFrames) {
                for (size_t j = startIndex; j < startIndex + numFrames; ++j) {
                    memory[j].occupied = true;
                    memory[j].processId = processId;
                    freeFrames.erase(std::remove(freeFrames.begin(), freeFrames.end(), j), freeFrames.end());
                }

                processAllocations[processId] = { startIndex, startIndex + numFrames - 1 };
                return true;
            }
        }
        else {
            consecutive = 0;
        }
    }

    return false;
}


void MemoryManager::deallocateFrames(size_t numFrames, size_t frameIndex, const std::vector<size_t>&) {

    for (size_t i = frameIndex; i < frameIndex + numFrames && i < memory.size(); ++i) {
        memory[i].occupied = false;
        memory[i].processId = -1;
        freeFrames.push_back(i);
    }

    for (auto it = processAllocations.begin(); it != processAllocations.end(); ) {
        if (it->second.first == frameIndex) {
            it = processAllocations.erase(it);
        }
        else {
            ++it;
        }
    }
}

size_t MemoryManager::getProcessStartFrame(int processId) const {
    auto it = processAllocations.find(processId);
    if (it != processAllocations.end()) return it->second.first;
    return -1;
}

void MemoryManager::printMemoryMap() const {
    std::cout << "Memory Map:\n";
    for (size_t i = 0; i < memory.size(); ++i) {
        std::cout << "[" << i << "] ";
        if (memory[i].occupied)
            std::cout << "PID " << memory[i].processId;
        else
            std::cout << "Free";
        std::cout << "\n";
    }
}