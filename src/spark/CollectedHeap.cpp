//
// Created by kiva on 2018-12-30.
//
#include <spark/CollectedHeap.h>
#include <exception>

namespace spark {

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

    CollectedHeap::CollectedHeap(Addr heapStart, Size heapSize)
        : heapStart(heapStart), heapSize(heapSize),
          heapUnUsedStart(heapStart), heapUnusedSize(heapSize) {
        createBlockTree();
    }

    void CollectedHeap::createBlockTree() {
        if (heapSize < SPARK_GC_HEAP_BLOCK) {
            throw std::exception();
        }

        Size unusedMaxSize = static_cast<Size>(heapSize * SPARK_GC_HEAP_UNUSED_FACTOR);
        while (heapUnusedSize > unusedMaxSize) {
            auto block = newBlockFromUnused(SPARK_GC_HEAP_BLOCK);
            heapBlocks.push_back(block);
        }
        heapBlocks.sort();
    }

    void CollectedHeap::dumpHeap(FILE *file) {
        fprintf(file, "CollectedHeap\n");
        fprintf(file, "\tHeap start address: %p\n", (void *) getHeapStart());
        fprintf(file, "\tHeap end address  : %p\n", (void *) getHeapEnd());
        fprintf(file, "\tHeap size in bytes: %zd\n", getHeapSize());
        fprintf(file, "\tHeap pre-sized chunks: %zd(%zd blocks)\n",
            getHeapUsed(), heapBlocks.size());
        fprintf(file, "\tHeap unused size     : %zd\n", getHeapUnusedSize());
        fprintf(file, "\tHeap unused start    : %p\n", (void *) heapUnUsedStart);
        fprintf(file, "\n");
        fprintf(file, "Heap pre-sized chunk details: %zd(%zd blocks)\n",
            getHeapUsed(), heapBlocks.size());

        int index = 0;
        for (auto heapBlock : heapBlocks) {
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
        for (auto heapBlock : heapBlocks) {
            if (heapBlock->canAfford(size)) {
                bestFits.push_back(heapBlock);
            }
        }

        if (!bestFits.empty()) {
            bestFits.sort();
            HeapBlock *selected = bestFits.front();
            return selected->allocate(size);
        }
        return nullptr;
    }

    Addr CollectedHeap::allocateLarge(Size size) {
        auto newBlock = newBlockFromUnused(isSuperObject(size) ? size : SPARK_GC_HEAP_BLOCK);
        if (newBlock == nullptr) {
            return nullptr;
        }

        Addr obj = newBlock->allocate(size);
        heapBlocks.push_back(newBlock);

        if (newBlock->getRemaining() > 0) {
            auto remaining = newBlock->shrinkToFit();
            heapBlocks.push_back(remaining);
        }

        heapBlocks.sort();
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
}
