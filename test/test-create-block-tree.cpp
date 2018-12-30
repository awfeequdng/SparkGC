//
// Created by kiva on 2018-12-30.
//
#include <spark/CollectedHeap.h>
#include <cstdlib>
#include <cstdio>

int main() {
    using namespace spark;

    Size memorySize = 4096;
    Addr memory = (Addr) malloc(memorySize);
    if (memory == nullptr) {
        return EXIT_FAILURE;
    }

    auto heap = new CollectedHeap(memory, memorySize);
    heap->dumpHeap(stdout);
    delete heap;
    return EXIT_SUCCESS;
}

