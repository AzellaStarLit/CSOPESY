#pragma once

#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <fstream>
#include <string>

class Process;

class MemoryManager {
private:
    // --- physical memory layout ---
    const size_t maxMemorySize;
    const size_t frameSize;
    const size_t numFrames;

    struct MemoryFrame {
        bool   occupied = false;
        int    processId = -1;
        size_t pageNum = 0;   // optional (not required for correctness)
		char* data = nullptr; // pointer to the actual data in this frame (allocated on demand)
    };

    std::vector<MemoryFrame> memory;
    std::vector<size_t>      freeFrames;                 // indices of free frames
    std::queue<size_t>       pageLoadOrder;              // FIFO of frame indices
    // frameIndex -> (pid, pageNum)
    std::unordered_map<size_t, std::pair<int, size_t>> reversePageMap;

    // Optional legacy map used by your snapshot function (kept for compatibility)
    std::unordered_map<int, std::pair<size_t, size_t>> processAllocations;

    size_t totalPageIns = 0;
    size_t totalPageOuts = 0;

	// --- page fault handling ---
    std::atomic<bool> lastPageFaultOccurred = false;

    // --- single backing store ---
    std::string  swapFilePath = "csopesy-backing-store.txt";
    std::fstream swapfile;

    // Synchronization:
    std::mutex mmMutex;    // protects memory, freeFrames, pageLoadOrder, reversePageMap, totals
    std::mutex swapMutex;  // protects swapfile I/O and swap bookkeeping below

    // swap bookkeeping (pid -> contiguous page region in the swap file)
    std::unordered_map<int, size_t> swapStartPage;
    std::unordered_map<int, size_t> swapNumPages;
    size_t nextFreeSwapPage = 0;
    size_t swapFileBytes = 0;

    // swap helpers (call only while holding swapMutex)
    void   ensureSwapSize(size_t requiredBytes);
    size_t swapOffsetBytes(int pid, size_t pageNum) const;
    void   writePage_nolock(int pid, size_t pageNum, const char* data);
    void   readPage_nolock(int pid, size_t pageNum, char* out);

public:
    MemoryManager(size_t maxMem, size_t frameSz);

    // demand paging
    bool   handlePageFault(Process* process, size_t pageNum);

    // optional APIs (kept for your existing code paths / snapshot)
    size_t getProcessStartFrame(int processId) const; // returns start frame if legacy contiguous allocation was used, else (size_t)-1
    void   printMemoryMap() const;

    void   snapshotMemoryToFile(int quantumCycle);
    size_t calculateExternalFragmentation() const;
    size_t countProcessesInMemory() const;

    // stats
    size_t getTotalPageIns()  const { return totalPageIns; }
    size_t getTotalPageOuts() const { return totalPageOuts; }

    // physical memory info
    size_t getUsedFrames() const;    // frames in use
    size_t getFreeFrames() const;    // frames free
    size_t getFrameSize()  const { return frameSize; }

    // swap registration
    void registerProcessSwap(int pid, size_t pagesNeeded);
    void unregisterProcessSwap(int pid);

    // convenience for cleanup on process exit (optional)
    void freeAllFramesForPid(int pid);

    // read/write memory
    bool writeUInt16(int pid, uint32_t address, uint16_t value);
    bool readUInt16(int pid, uint32_t address, uint16_t& outValue);

    bool readByte(int pid, size_t virtualAddress, char& outByte);
    bool writeByte(int pid, size_t virtualAddress, char inByte);

    bool translate(int pid, size_t virtualAddress, size_t& physicalAddress);

    bool wasPageFault() const;
};
