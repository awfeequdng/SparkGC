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

        int getAllocateCounter() const {
            return allocateCounter;
        }

        bool canAfford(Size size) const {
            return getRemaining() >= size;
        }

        bool inBlock(const Addr addr) const {
            return addr >= getStart() && addr < getEnd();
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

        bool operator<(const HeapBlock &other) const;

        HeapBlock *shrinkToFit();

        void reset();
    };

    class CollectedHeap {
        friend class SparkGC;

    public:
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
        template<typename T>
        static bool inRange(T t, T low, T high) {
            return t > low && t <= high;
        }

    public:
        static Size align(Size size) {
            constexpr Size base = SPARK_GC_HEAP_SMALL;
            return ((size + base - 1) & (~(base - 1)));
        }

        static bool isSmallObject(Size size) {
            return inRange<Size>(size, 0, SPARK_GC_HEAP_SMALL);
        }

        static bool isMediumObject(Size size) {
            return inRange<Size>(size, SPARK_GC_HEAP_SMALL, SPARK_GC_HEAP_MEDIUM);
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
        Tree<HeapBlock *> partiallyFreeBlocks;
        Tree<HeapBlock *> fullBlocks;

    private:
        Addr allocateLarge(Size size);

        HeapBlock *newBlockFromUnused(Size size);

        void createBlockTree();

        void sort(Tree<HeapBlock *> &blocks);

    protected:
        void memoryFreed(const Tree<CollectedObject*> &free);

    public:
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
            return partiallyFreeBlocks.size();
        }

        Size getMaxBlockCount() const noexcept {
            return (heapSize / SPARK_GC_HEAP_BLOCK);
        }

        Addr allocate(Size rawSize);

        void dumpHeap(FILE *file);
    };
}
