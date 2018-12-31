//
// Created by kiva on 2018/4/19.
//
#pragma once

#include <spark/SparkGC_Shared.h>
#include <cstdio>
#include <list>

namespace spark {
    class HeapBlock {
    private:
        Size blockSize;
        Addr blockStart;
        Addr current;
        int allocateCounter;

    public:
        HeapBlock(Addr blockStart, Size blockSize);

        Addr getStart() const {
            return blockStart;
        }

        Addr getEnd() const {
            return blockStart + blockSize;
        }

        Size getSize() const {
            return blockSize;
        }

        Size getUsed() const {
            return current - blockStart;
        }

        Size getRemaining() const {
            return blockSize - getUsed();
        }

        bool canAfford(Size size) const {
            return getRemaining() >= size;
        }

        Addr allocate(Size size) {
            if (canAfford(size)) {
                Addr obj = current;
                current += size;
                ++allocateCounter;
                return obj;
            }
            return nullptr;
        }

        bool operator<(const HeapBlock &other) {
            return (getRemaining() == other.getRemaining()) ?
                   (blockStart < other.blockStart) :
                   (getRemaining() < other.getRemaining());
        }

        int getAllocateCounter() const {
            return allocateCounter;
        }

        HeapBlock *shrinkToFit();
    };

    class CollectedHeap {
        template<typename T>
        using Tree = std::list<T>;

    private:
        /**
         * Whether a size belongs to (low, high]
         * @param size size to check
         * @param low low size
         * @param high high size
         * @return true if it belongs to
         */
        static bool inRange(Size size, Size low, Size high) {
            return size > low && size <= high;
        }

        static bool isSmallObject(Size size) {
            return inRange(size, 0, SPARK_GC_HEAP_SMALL);
        }

        static bool isMediumObject(Size size) {
            return inRange(size, SPARK_GC_HEAP_SMALL, SPARK_GC_HEAP_MEDIUM);
        }

        static bool isLargeObject(Size size) {
            return size > SPARK_GC_HEAP_MEDIUM;
        }

        static bool isSuperObject(Size size) {
            return size > SPARK_GC_HEAP_BLOCK;
        }

    private:
        Size heapSize;
        Size heapUnusedSize;
        Addr heapStart;
        Addr heapUnUsedStart;

    private:
        Addr allocateLarge(Size size);

        HeapBlock *newBlockFromUnused(Size size);

        void createBlockTree();

    public:
        Tree<HeapBlock *> heapBlocks;

        CollectedHeap(Addr heapStart, Size heapSize);

        ~CollectedHeap() = default;

        Size getHeapSize() const noexcept {
            return heapSize;
        }

        Addr getHeapStart() const noexcept {
            return heapStart;
        }

        Addr getHeapEnd() const noexcept {
            return heapStart + heapSize;
        }

        Size getHeapUsed() const noexcept {
            return heapSize - heapUnusedSize;
        }

        Size getHeapUnusedSize() const noexcept {
            return heapUnusedSize;
        }

        Size getBlockCount() const noexcept {
            return heapBlocks.size();
        }

        Size getMaxBlockCount() const noexcept {
            return (heapSize / SPARK_GC_HEAP_BLOCK);
        }

        Addr allocate(Size size);

        void dumpHeap(FILE *file);
    };
}
