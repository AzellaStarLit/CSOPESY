#include "MemoryManager.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <filesystem>


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

    if (freeFrames.size() < numFrames) return false;

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

size_t MemoryManager::calculateExternalFragmentation() const {
    size_t fragments = 0;
    bool insideFragment = false;

    for (const auto& frame : memory) {
        if (!frame.occupied) {
            if (!insideFragment) {
                insideFragment = true;
            }
            fragments++;
        } else {
            insideFragment = false;
        }
    }

    size_t freeBytes = 0;
    size_t currentFreeBlock = 0;
    int blockCount = 0;

    for (const auto& frame : memory) {
        if (!frame.occupied) {
            currentFreeBlock++;
        } else {
            if (currentFreeBlock > 0) {
                freeBytes += currentFreeBlock * frameSize;
                blockCount++;
                currentFreeBlock = 0;
            }
        }
    }

    if (currentFreeBlock > 0) {
        freeBytes += currentFreeBlock * frameSize;
        blockCount++;
    }

    return (blockCount >= 2) ? freeBytes : 0;
}


size_t MemoryManager::countProcessesInMemory() const {
    return processAllocations.size();
}

void MemoryManager::snapshotMemoryToFile(int quantumCycle) {
    std::filesystem::create_directory("memory stamp");

    std::ostringstream filename;
    filename << "memory stamp/memory_stamp_" << quantumCycle << ".txt";
    std::ofstream out(filename.str());

    if (!out.is_open()) return;

    // Timestamp
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    out << "Timestamp: (" << std::put_time(&tm, "%m/%d/%Y %I:%M:%S%p") << ")\n";

    size_t numProcs = countProcessesInMemory();
    size_t extFrag = calculateExternalFragmentation();

    out << "Number of processes in memory: " << numProcs
        << " \nTotal external fragmentation in KB: " << (extFrag / 1024) << "\n\n";

    out << "----end---- = " << maxMemorySize << "\n";

    std::vector<std::pair<int, std::pair<size_t, size_t>>> sortedAllocations(
        processAllocations.begin(), processAllocations.end()
    );

    std::sort(sortedAllocations.begin(), sortedAllocations.end(), [](const auto& a, const auto& b) {
        return a.second.second > b.second.second; // sort by end frame descending
        });

    for (const auto& [pid, bounds] : sortedAllocations) {
        size_t upper = (bounds.second + 1) * frameSize;
        size_t lower = bounds.first * frameSize;

        out << upper << "\n";
        out << "P" << pid << "\n";
        out << lower << "\n\n";
    }

    out << "----start----- = 0\n";
}