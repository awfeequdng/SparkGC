//
// Created by kiva on 2018-12-30.
//
#include <spark/CollectedHeap.h>
#include <spark/CollectedObject.h>
#include <spark/SparkGC.h>
#include <exception>
#include <vector>
#include <algorithm>


namespace spark {
    static bool comparator(HeapBlock *lhs, HeapBlock *rhs) {
        return (*lhs) < (*rhs);
    }

    static bool comparatorAddress(HeapBlock *lhs, HeapBlock *rhs) {
        return lhs->getStart() < rhs->getStart();
    }

    static HeapBlock *searchBlock(const std::vector<HeapBlock *> &all, Addr addr) {
        Size low = 0;
        Size high = all.size() - 1;
        Size mid;

        while (low <= high) {
            mid = (low + high) / 2;
            HeapBlock *block = all[mid];

            if (block->inBlock(addr)) {
                return block;
            }

            if (block->getStart() > addr) {
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        }
        return nullptr;
    }

    HeapBlock::HeapBlock(Addr blockStart, Size blockSize)
        : blockStart(blockStart), blockSize(blockSize), current(blockStart), allocateCounter(0) {
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

    void HeapBlock::reset() {
        current = blockStart;
        allocateCounter = 0;
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
        partiallyFreeBlocks.push_back(block);
        sort(partiallyFreeBlocks);
    }

    void CollectedHeap::dumpHeap(FILE *file) {
        fprintf(file, "CollectedHeap\n");
        fprintf(file, "\tHeap start address   : %p\n", (void *) getHeapStart());
        fprintf(file, "\tHeap end address     : %p\n", (void *) getHeapEnd());
        fprintf(file, "\tHeap size in bytes   : %zd\n", getHeapSize());
        fprintf(file, "\tHeap blocks          : %zd(%zd blocks)\n",
            getHeapUsed(), partiallyFreeBlocks.size() + fullBlocks.size());
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
        fprintf(file, "Heap free block details: %zd blocks\n", partiallyFreeBlocks.size());
        index = 0;
        for (auto heapBlock : partiallyFreeBlocks) {
            fprintf(file, "\tChunk %d:\n", index++);
            fprintf(file, "\t\tBlock start      : %p\n", (void *) heapBlock->getStart());
            fprintf(file, "\t\tBlock end        : %p\n", (void *) heapBlock->getEnd());
            fprintf(file, "\t\tBlock total size : %zd\n", heapBlock->getSize());
            fprintf(file, "\t\tBlock used size  : %zd\n", heapBlock->getUsed());
            fprintf(file, "\t\tBlock free size  : %zd\n", heapBlock->getRemaining());
            fprintf(file, "\t\tAllocate Counter : %d\n", heapBlock->getAllocateCounter());
        }
    }

    Addr CollectedHeap::allocate(Size rawSize) {
        Size size = align(rawSize);
        if (isLargeObject(size)) {
            return allocateLarge(size);
        }

        Tree<HeapBlock *> bestFits;
        for (auto heapBlock : partiallyFreeBlocks) {
            if (heapBlock->canAfford(size)) {
                bestFits.push_back(heapBlock);
            }
        }

        if (!bestFits.empty()) {
            sort(bestFits);
            auto selected = bestFits.front();
            Addr addr = selected->allocate(size);
            if (selected->getRemaining() < SPARK_GC_ALIGN) {
                partiallyFreeBlocks.remove(selected);
                fullBlocks.push_back(selected);
            }
            return addr;
        } else {
            auto newBlock = newBlockFromUnused(SPARK_GC_HEAP_BLOCK);
            if (newBlock == nullptr) {
                return nullptr;
            }
            Addr addr = newBlock->allocate(size);
            partiallyFreeBlocks.push_back(newBlock);
            return addr;
        }
    }

    Addr CollectedHeap::allocateLarge(Size size) {
        auto newBlock = newBlockFromUnused(isSuperObject(size) ? size : SPARK_GC_HEAP_BLOCK);
        if (newBlock == nullptr) {
            return nullptr;
        }

        Addr obj = newBlock->allocate(size);

        if (newBlock->getRemaining() >= SPARK_GC_ALIGN) {
            auto remaining = newBlock->shrinkToFit();
            partiallyFreeBlocks.push_back(remaining);
        }

        fullBlocks.push_back(newBlock);
        sort(partiallyFreeBlocks);
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

    void CollectedHeap::sort(Tree<HeapBlock *> &blocks) {
        blocks.sort(comparator);
    }

    void CollectedHeap::memoryFreed(SparkGC *gc, const Tree<CollectedObject *> &free) {
        Tree<HeapBlock *> newPartially;
        std::vector<HeapBlock *> all;
        Tree<HeapBlock *> deleteList;

        std::merge(fullBlocks.begin(), fullBlocks.end(),
            partiallyFreeBlocks.begin(), partiallyFreeBlocks.end(),
            std::back_inserter(all),
            comparatorAddress);

        for (auto object : free) {
            gc->setColor(Addr(object), GC_COLOR_BLUE);

            Size size = object->getOnStackSize();
            Addr start = Addr(object);
            Addr end = start + size;

            DEBUG(printf("*** SparkGC *** : Sweeping object %p (sized %zd)\n",
                object, size));

            auto block = searchBlock(all, start);
            if (block == nullptr) {
                throw std::exception();
            }

            all.erase(std::remove(all.begin(), all.end(), block), all.end());

            if (block->getRemaining() + size == block->getSize()) {
                block->reset();
                newPartially.push_back(block);

            } else {
                deleteList.push_back(block);
                auto objBlock = new HeapBlock(start, size);
                newPartially.push_back(objBlock);

                if (block->getStart() < start) {
                    auto leftBlock = new HeapBlock(block->getStart(),
                        start - block->getStart());
                    all.push_back(leftBlock);
                }

                if (end < block->getEnd()) {
                    auto rightBlock = new HeapBlock(end, block->getEnd() - end);
                    all.push_back(rightBlock);
                }

                std::sort(all.begin(), all.end(), comparatorAddress);
            }
        }

        fullBlocks.clear();
        for (auto block : all) {
            fullBlocks.push_back(block);
        }

        sort(newPartially);
        partiallyFreeBlocks.swap(newPartially);
    }
}
