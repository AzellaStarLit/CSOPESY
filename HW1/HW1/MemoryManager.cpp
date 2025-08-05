#include "MemoryManager.h"
#include "Process.h"
#include "ProcessManager.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <cstring>  // std::memset

extern ProcessManager processManager;

MemoryManager::MemoryManager(size_t maxMem, size_t frameSz)
    : maxMemorySize(maxMem),
    frameSize(frameSz),
    numFrames((frameSz == 0) ? 0 : maxMem / frameSz)
{
    memory.resize(numFrames);
    for (size_t i = 0; i < numFrames; ++i) {
        freeFrames.push_back(i);
    }

    // single backing store
    swapfile.open(swapFilePath, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    if (!swapfile.is_open()) {
        std::ofstream create(swapFilePath, std::ios::binary);
        create.close();
        swapfile.open(swapFilePath, std::ios::in | std::ios::out | std::ios::binary);
    }
    swapfile.seekp(0, std::ios::end);
    swapFileBytes = static_cast<size_t>(swapfile.tellp());
    if (!swapfile) swapfile.clear();
}

// -------------------------- Demand paging -------------------------- //

bool MemoryManager::handlePageFault(Process* process, size_t pageNum)
{
    // Serialize in-memory MM state
    std::lock_guard<std::mutex> mmLock(mmMutex);

    // already resident?
    auto& entry = process->getPageEntry(pageNum);
    if (entry.valid) return true;

    auto load_into_frame = [&](size_t frameIndex) {
        memory[frameIndex].occupied = true;
        memory[frameIndex].processId = process->getPID();

        /*
        std::string pageData(frameSize, '\0');
        {
            std::lock_guard<std::mutex> swLock(swapMutex);
            readPage_nolock(process->getPID(), pageNum, pageData.data());
        }
        */

        // Allocate data buffer if not allocated yet
        if (memory[frameIndex].data == nullptr) {
            memory[frameIndex].data = new char[frameSize];
        }

        // Read the page from swap into frame's data buffer
        {
            std::lock_guard<std::mutex> swLock(swapMutex);
            readPage_nolock(process->getPID(), pageNum, memory[frameIndex].data);
        }


        ++totalPageIns;
        process->incrementPageIns();

        entry.valid = true;
        entry.frameNumber = frameIndex;
        entry.dirty = false;

        reversePageMap[frameIndex] = { process->getPID(), pageNum };
        pageLoadOrder.push(frameIndex);
        };

    size_t targetFrame = SIZE_MAX;

    if (freeFrames.empty())
    {
        if (pageLoadOrder.empty()) {
            std::cerr << "Error: no frame to evict and no load order tracked.\n";
            return false;
        }

        // FIFO eviction
        size_t victimFrame = pageLoadOrder.front();
        pageLoadOrder.pop();

        auto it = reversePageMap.find(victimFrame);
        if (it != reversePageMap.end()) {
            int    victimPid = it->second.first;
            size_t victimPageNum = it->second.second;

            if (Process* victimProc = processManager.get_process_by_pid(victimPid)) {
                auto& ventry = victimProc->getPageEntry(victimPageNum);
                if (ventry.valid) {
                    if (ventry.dirty) {
                        std::string data(frameSize, '\0'); // (simulate payload)
                        std::lock_guard<std::mutex> swLock(swapMutex);
                        //writePage_nolock(victimPid, victimPageNum, data.data());
                        writePage_nolock(victimPid, victimPageNum, memory[victimFrame].data);
                        ++totalPageOuts;
                        victimProc->incrementPageOuts();
                    }
                    ventry.valid = false;
                    ventry.frameNumber = static_cast<size_t>(-1);
                }
            }
            reversePageMap.erase(it);
        }

        // reuse the victim frame for the faulting page

        //these 3 lines are added
		// Free the data buffer if it exists
        if (memory[victimFrame].data != nullptr) {
            delete[] memory[victimFrame].data;
            memory[victimFrame].data = nullptr;
        }

        memory[victimFrame].occupied = false;
        memory[victimFrame].processId = -1;
        targetFrame = victimFrame;
    }
    else
    {
        targetFrame = freeFrames.back();
        freeFrames.pop_back();
    }

    load_into_frame(targetFrame);
    return true;
}

// -------------------------- Optional helpers / legacy APIs -------------------------- //

size_t MemoryManager::getProcessStartFrame(int processId) const {
    auto it = processAllocations.find(processId);
    if (it != processAllocations.end()) return it->second.first;
    return static_cast<size_t>(-1);
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
    // (kept as in your version)
    size_t freeBytes = 0;
    size_t currentFreeBlock = 0;
    int    blockCount = 0;

    for (const auto& frame : memory) {
        if (!frame.occupied) {
            currentFreeBlock++;
        }
        else {
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

    std::sort(sortedAllocations.begin(), sortedAllocations.end(),
        [](const auto& a, const auto& b) {
            return a.second.second > b.second.second; // end frame descending
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

size_t MemoryManager::getUsedFrames() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mmMutex));
    return numFrames - freeFrames.size();
}

size_t MemoryManager::getFreeFrames() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mmMutex));
    return freeFrames.size();
}

// -------------------------- swap helpers -------------------------- //

void MemoryManager::ensureSwapSize(size_t requiredBytes) {
    if (requiredBytes <= swapFileBytes) return;
    swapfile.seekp(requiredBytes - 1, std::ios::beg);
    char zero = '\0';
    swapfile.write(&zero, 1);
    swapfile.flush();
    swapFileBytes = requiredBytes;
}

size_t MemoryManager::swapOffsetBytes(int pid, size_t pageNum) const {
    auto it = swapStartPage.find(pid);
    if (it == swapStartPage.end()) return SIZE_MAX;
    size_t startPage = it->second;
    return (startPage + pageNum) * frameSize;
}

void MemoryManager::writePage_nolock(int pid, size_t pageNum, const char* data) {
    size_t off = swapOffsetBytes(pid, pageNum);
    if (off == SIZE_MAX) return;
    ensureSwapSize(off + frameSize);
    swapfile.seekp(off, std::ios::beg);
    swapfile.write(data, frameSize);
    swapfile.flush();
}

void MemoryManager::readPage_nolock(int pid, size_t pageNum, char* out) {
    size_t off = swapOffsetBytes(pid, pageNum);
    if (off == SIZE_MAX) { std::memset(out, 0, frameSize); return; }
    ensureSwapSize(off + frameSize);
    swapfile.seekg(off, std::ios::beg);
    swapfile.read(out, frameSize);

    if (swapfile.gcount() < static_cast<std::streamsize>(frameSize)) {
        std::streamsize got = swapfile.gcount();
        if (got < 0) got = 0;
        std::memset(out + got, 0, frameSize - static_cast<size_t>(got));
        swapfile.clear();
    }
}

// -------------------------- swap registration -------------------------- //

void MemoryManager::registerProcessSwap(int pid, size_t pagesNeeded) {
    std::lock_guard<std::mutex> swLock(swapMutex);
    if (swapStartPage.count(pid)) return;
    size_t start = nextFreeSwapPage;
    nextFreeSwapPage += pagesNeeded;
    swapStartPage[pid] = start;
    swapNumPages[pid] = pagesNeeded;
    ensureSwapSize(nextFreeSwapPage * frameSize);
}

void MemoryManager::unregisterProcessSwap(int pid) {
    std::lock_guard<std::mutex> swLock(swapMutex);
    swapStartPage.erase(pid);
    swapNumPages.erase(pid);
}

// -------------------------- optional cleanup -------------------------- //

void MemoryManager::freeAllFramesForPid(int pid) {
    std::lock_guard<std::mutex> mmLock(mmMutex);
    for (auto it = reversePageMap.begin(); it != reversePageMap.end(); ) {
        if (it->second.first == pid) {
            size_t f = it->first;

            if (memory[f].data != nullptr) {
                delete[] memory[f].data;
                memory[f].data = nullptr;
            }

            memory[f].occupied = false;
            memory[f].processId = -1;
            freeFrames.push_back(f);
            it = reversePageMap.erase(it);
        }
        else {
            ++it;
        }
    }
    // purge any stale entries at the front of FIFO
    while (!pageLoadOrder.empty() && !memory[pageLoadOrder.front()].occupied)
        pageLoadOrder.pop();
}

// Logic for memory read/write operations

bool MemoryManager::readByte(int pid, size_t virtualAddress, char& outByte) {
    size_t physicalAddress;
    if (!translate(pid, virtualAddress, physicalAddress)) {
        return false;
    }

    size_t frameNum = physicalAddress / frameSize;
    size_t offset = physicalAddress % frameSize;

    if (frameNum >= memory.size() || !memory[frameNum].data) {
        std::cerr << "readByte: Frame data missing\n";
        return false;
    }

    outByte = memory[frameNum].data[offset];
    return true;
}


bool MemoryManager::writeByte(int pid, size_t virtualAddress, char inByte) {
    size_t physicalAddress;
    if (!translate(pid, virtualAddress, physicalAddress)) {
        return false;
    }

    size_t frameNum = physicalAddress / frameSize;
    size_t offset = physicalAddress % frameSize;

    if (frameNum >= memory.size() || !memory[frameNum].data) {
        std::cerr << "writeByte: Frame data missing\n";
        return false;
    }

    memory[frameNum].data[offset] = inByte;

    Process* proc = processManager.get_process_by_pid(pid);
    if (Process* proc = processManager.get_process_by_pid(pid)) {
        size_t pageNum = virtualAddress / frameSize;
        proc->getPageEntry(pageNum).dirty = true;
    }

    return true;
}


bool MemoryManager::translate(int pid, size_t virtualAddress, size_t& physicalAddress) {
    //lastPageFaultOccurred = false;  // reset

    Process* proc = processManager.get_process_by_pid(pid);
    if (!proc) return false;

    size_t pageNum = virtualAddress / frameSize;
    size_t offset = virtualAddress % frameSize;

    if (pageNum >= proc->pageTable.size()) return false;

    auto& pageEntry = proc->getPageEntry(pageNum);

    if (!pageEntry.valid) {
        // Page fault occurred
        if (!handlePageFault(proc, pageNum)) {
            std::cerr << "translate: Page fault handling failed\n";
            return false;
        }
        lastPageFaultOccurred = true; // mark that it happened!
    }

    size_t frameNum = pageEntry.frameNumber;
    if (frameNum >= memory.size()) return false;

    physicalAddress = frameNum * frameSize + offset;
    if (physicalAddress >= memory.size() * frameSize) return false;

    return true;
}


bool MemoryManager::wasPageFault() const {
    return lastPageFaultOccurred;
}

void MemoryManager::resetPageFaultFlag() {
    lastPageFaultOccurred = false; // reset the flag
}

// Already defined function — keep this
bool MemoryManager::writeUInt16(int pid, uint32_t virtualAddress, uint16_t value) {
    char lowByte = static_cast<char>(value & 0xFF);
    char highByte = static_cast<char>((value >> 8) & 0xFF);

    if (!writeByte(pid, virtualAddress, lowByte)) return false;
    if (!writeByte(pid, virtualAddress + 1, highByte)) return false;

    return true;
}


bool MemoryManager::readUInt16(int pid, uint32_t virtualAddress, uint16_t& outValue) {
    char lowByte = 0, highByte = 0;

    if (!readByte(pid, virtualAddress, lowByte)) return false;
    if (!readByte(pid, virtualAddress + 1, highByte)) return false;

    outValue = (static_cast<uint8_t>(highByte) << 8) | static_cast<uint8_t>(lowByte);
    return true;
}



