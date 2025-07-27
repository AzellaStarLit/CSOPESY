#pragma once

#include <unordered_map>
#include <vector>
#include "Process.h"
#include <queue>

class Process;

class MemoryManager {
private:
    const size_t maxMemorySize;
    const size_t frameSize;
    const size_t numFrames;

    std::unordered_map<size_t, size_t> frameMap;
    std::vector<size_t> freeFrames;

    // Each memory frame
    struct MemoryFrame {
        bool occupied = false;
        int processId = -1;
        size_t pageNum;
    };

    

    std::unordered_map<size_t, std::pair<int, size_t>> reversePageMap; // frameIndex -> (pid, pageNum)
    std::queue<size_t> pageLoadOrder; // For FIFO eviction

    std::vector<MemoryFrame> memory;
    std::unordered_map<int, std::pair<size_t, size_t>> processAllocations; // pid (startFrame, endFrame)

    size_t totalPageIns = 0;
    size_t totalPageOuts = 0;


public:
    //MemoryManager();
    MemoryManager(size_t maxMem, size_t frameSz);

    bool allocateFrames(size_t numFrames, size_t processId, const std::vector<size_t>& pageSize);
    void deallocateFrames(size_t numFrames, size_t frameIndex, const std::vector<size_t>& pageSize);

    //get start frame of process
    size_t getProcessStartFrame(int processId) const;

    //debug
    void printMemoryMap() const; 

    void snapshotMemoryToFile(int quantumCycle);
    size_t calculateExternalFragmentation() const;
    size_t countProcessesInMemory() const;

    // In MemoryManager.h
    bool handlePageFault(Process* process, size_t pageNum);

    // --- page?fault statistics ---
    size_t getTotalPageIns()  const { return totalPageIns; }
    size_t getTotalPageOuts() const { return totalPageOuts; }

};