//
// Created by kiva on 2018-12-30.
//
#include <spark/CollectedHeap.h>
#include <exception>

namespace spark {

    HeapBlock::HeapBlock(Addr blockStart, Size blockSize)
        : blockStart(blockStart), blockSize(blockSize), current(blockStart) {
    }

    CollectedHeap::CollectedHeap(Addr heapStart, Size heapSize)
        : heapStart(heapStart), heapSize(heapSize) {
        createBlockTree();
    }

    void CollectedHeap::createBlockTree() {
        if (heapSize < SPARK_GC_HEAP_BLOCK) {
            throw std::exception();
        }

        heapUnusedSize = static_cast<Size>(heapSize * SPARK_GC_HEAP_UNUSED_FACTOR);
        heapUnUsedStart = heapStart + (heapSize - heapUnusedSize);

        Addr current = heapStart;
        while (current < heapUnUsedStart) {
            auto block = new HeapBlock(current, SPARK_GC_HEAP_BLOCK);
            heapBlocks.push_back(block);
            current += SPARK_GC_HEAP_BLOCK;
        }

        heapUnUsedStart = current;
        heapUnusedSize = getHeapEnd() - current;
        heapBlocks.sort();
    }

    void CollectedHeap::dumpHeap(FILE *file) {
        fprintf(file, "CollectedHeap\n");
        fprintf(file, "\tHeap start address: %p\n", (void *) heapStart);
        fprintf(file, "\tHeap size in bytes: %zd\n", heapSize);
        fprintf(file, "\tHeap pre-sized chunks: %zd(%zd blocks)\n",
            heapSize - heapUnusedSize, heapBlocks.size());
        fprintf(file, "\tHeap unused size     : %zd\n", heapUnusedSize);
        fprintf(file, "\tHeap unused start    : %p\n", (void *) heapUnUsedStart);
        fprintf(file, "\n");
        fprintf(file, "Heap pre-sized chunk details: %zd(%zd blocks)\n",
            heapSize - heapUnusedSize, heapBlocks.size());

        int index = 0;
        for (auto heapBlock : heapBlocks) {
            fprintf(file, "\tChunk %d:\n", index++);
            fprintf(file, "\t\tBlock start      : %p\n", (void *) heapBlock->getStart());
            fprintf(file, "\t\tBlock end        : %p\n", (void *) heapBlock->getEnd());
            fprintf(file, "\t\tBlock total size : %zd\n", heapBlock->getSize());
            fprintf(file, "\t\tBlock used size  : %zd\n", heapBlock->getUsed());
            fprintf(file, "\t\tBlock free size  : %zd\n", heapBlock->getRemaining());
        }
    }
}
