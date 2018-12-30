//
// Created by kiva on 2018/4/19.
//
#pragma once

#include <spark/SparkGC_Shared.h>
#include <cstdio>
#include <list>

/**
 * sizeof(intOopDesc)
 */
#define SPARK_GC_HEAP_SMALL 32

/**
 * sizeof(instanceOopDesc)
 */
#define SPARK_GC_HEAP_MEDIUM 48

#define SPARK_GC_HEAP_BLOCK 192

#define SPARK_GC_HEAP_UNUSED_FACTOR (0.4f)

namespace spark {
    class HeapBlock {
    private:
        Size blockSize;
        Addr blockStart;
        Addr current;

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
                return obj;
            }
            return nullptr;
        }

        bool operator<(const HeapBlock &other) {
            return (blockSize == other.blockSize) ?
                   (blockStart < other.blockStart) :
                   (blockSize < other.blockSize);
        }
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
            return inRange(size, SPARK_GC_HEAP_MEDIUM, SPARK_GC_HEAP_BLOCK);
        }

    public:
        Size heapSize;
        Size heapUnusedSize;
        Addr heapStart;
        Addr heapUnUsedStart;

        Tree<HeapBlock *> heapBlocks;

        CollectedHeap(Addr heapStart, Size heapSize);

        ~CollectedHeap() = default;

        Size getHeapSize() const noexcept {
            return heapSize;
        }

        Addr getHeapStart() noexcept {
            return heapStart;
        }

        Addr getHeapEnd() noexcept {
            return heapStart + heapSize;
        }

        void createBlockTree();

        void dumpHeap(FILE *file);
    };
}