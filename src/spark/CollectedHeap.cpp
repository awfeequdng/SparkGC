//
// Created by kiva on 2018-12-30.
//
#include <spark/CollectedHeap.h>
#include <exception>

namespace spark {
    static bool comparator(HeapBlock *lhs, HeapBlock *rhs) {
        return (*lhs) < (*rhs);
    }

    HeapBlock::HeapBlock(Addr blockStart, Size blockSize)
        : blockStart(blockStart), blockSize(blockSize), current(blockStart) {
        allocateCounter = 0;
    }

    HeapBlock *HeapBlock::shrinkToFit() {
        Size size = getRemaining();
        Addr start = current;
        auto block = new HeapBlock(start, size);
        blockSize = current - blockStart;
        return block;
    }

    bool HeapBlock::operator<(const HeapBlock &other) const {
        return (getRemaining() == other.getRemaining()) ?
               (blockStart < other.blockStart) :
               (getRemaining() < other.getRemaining());
    }

    CollectedHeap::CollectedHeap(Addr heapStart, Size heapSize)
        : heapStart(heapStart), heapSize(heapSize),
          heapUnUsedStart(heapStart), heapUnusedSize(heapSize) {
        createBlockTree();
    }

    void CollectedHeap::createBlockTree() {
        if (heapSize < SPARK_GC_HEAP_BLOCK) {
            throw std::exception();
        }

        // Initially there is only a single block
        auto block = newBlockFromUnused(SPARK_GC_HEAP_BLOCK);
        freeBlocks.push_back(block);
        sort(freeBlocks);
    }

    void CollectedHeap::dumpHeap(FILE *file) {
        fprintf(file, "CollectedHeap\n");
        fprintf(file, "\tHeap start address: %p\n", (void *) getHeapStart());
        fprintf(file, "\tHeap end address  : %p\n", (void *) getHeapEnd());
        fprintf(file, "\tHeap size in bytes: %zd\n", getHeapSize());
        fprintf(file, "\tHeap blocks       : %zd(%zd blocks)\n",
            getHeapUsed(), freeBlocks.size() + fullBlocks.size());
        fprintf(file, "\tHeap unused size     : %zd\n", getHeapUnusedSize());
        fprintf(file, "\tHeap unused start    : %p\n", (void *) heapUnUsedStart);
        fprintf(file, "\n");

        int index = 0;
        fprintf(file, "Heap full block details: %zd blocks\n", fullBlocks.size());
        for (auto heapBlock : fullBlocks) {
            fprintf(file, "\tChunk %d:\n", index++);
            fprintf(file, "\t\tBlock start      : %p\n", (void *) heapBlock->getStart());
            fprintf(file, "\t\tBlock end        : %p\n", (void *) heapBlock->getEnd());
            fprintf(file, "\t\tBlock total size : %zd\n", heapBlock->getSize());
            fprintf(file, "\t\tBlock used size  : %zd\n", heapBlock->getUsed());
            fprintf(file, "\t\tBlock free size  : %zd\n", heapBlock->getRemaining());
            fprintf(file, "\t\tAllocate Counter : %d\n", heapBlock->getAllocateCounter());
        }

        fprintf(file, "\n\n\n");
        fprintf(file, "Heap free block details: %zd blocks\n", freeBlocks.size());
        index = 0;
        for (auto heapBlock : freeBlocks) {
            fprintf(file, "\tChunk %d:\n", index++);
            fprintf(file, "\t\tBlock start      : %p\n", (void *) heapBlock->getStart());
            fprintf(file, "\t\tBlock end        : %p\n", (void *) heapBlock->getEnd());
            fprintf(file, "\t\tBlock total size : %zd\n", heapBlock->getSize());
            fprintf(file, "\t\tBlock used size  : %zd\n", heapBlock->getUsed());
            fprintf(file, "\t\tBlock free size  : %zd\n", heapBlock->getRemaining());
            fprintf(file, "\t\tAllocate Counter : %d\n", heapBlock->getAllocateCounter());
        }
    }

    Addr CollectedHeap::allocate(Size size) {
        if (isLargeObject(size)) {
            return allocateLarge(size);
        }

        Tree<HeapBlock *> bestFits;
        for (auto heapBlock : freeBlocks) {
            if (heapBlock->canAfford(size)) {
                bestFits.push_back(heapBlock);
            }
        }

        if (!bestFits.empty()) {
            sort(bestFits);
            auto selected = bestFits.front();
            Addr addr = selected->allocate(size);
            if (selected->getRemaining() < SPARK_GC_HEAP_SMALL) {
                freeBlocks.remove(selected);
                fullBlocks.push_back(selected);
            }
            return addr;
        } else {
            auto newBlock = newBlockFromUnused(SPARK_GC_HEAP_BLOCK);
            if (newBlock == nullptr) {
                return nullptr;
            }
            Addr addr = newBlock->allocate(size);
            freeBlocks.push_back(newBlock);
            return addr;
        }
    }

    Addr CollectedHeap::allocateLarge(Size size) {
        auto newBlock = newBlockFromUnused(isSuperObject(size) ? size : SPARK_GC_HEAP_BLOCK);
        if (newBlock == nullptr) {
            return nullptr;
        }

        Addr obj = newBlock->allocate(size);

        if (newBlock->getRemaining() >= SPARK_GC_HEAP_SMALL) {
            auto remaining = newBlock->shrinkToFit();
            freeBlocks.push_back(remaining);
        }

        fullBlocks.push_back(newBlock);
        sort(freeBlocks);
        return obj;
    }

    HeapBlock *CollectedHeap::newBlockFromUnused(Size size) {
        if (heapUnusedSize >= size) {
            auto block = new HeapBlock(heapUnUsedStart, size);
            heapUnusedSize -= size;
            heapUnUsedStart += size;
            return block;
        }
        return nullptr;
    }

    void CollectedHeap::reblock() {
        // TODO
    }

    void CollectedHeap::sort(Tree<HeapBlock *> &blocks) {
        blocks.sort(comparator);
    }
}
