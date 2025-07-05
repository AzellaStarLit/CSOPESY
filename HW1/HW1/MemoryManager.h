#pragma once

#include <unordered_map>
#include <vector>

class MemoryManager {
private:
    static const size_t maxMemorySize;
    static const size_t frameSize;
    static const size_t numFrames;

    std::unordered_map<size_t, size_t> frameMap;
    std::vector<size_t> freeFrames;

    // Each memory frame
    struct MemoryFrame {
        bool occupied = false;
        int processId = -1;
    };

    std::vector<MemoryFrame> memory;
    std::unordered_map<int, std::pair<size_t, size_t>> processAllocations; // pid (startFrame, endFrame)

public:
    MemoryManager();

    bool allocateFrames(size_t numFrames, size_t processId, const std::vector<size_t>& pageSize);
    void deallocateFrames(size_t numFrames, size_t frameIndex, const std::vector<size_t>& pageSize);
};