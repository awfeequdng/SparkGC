//
// Created by kiva on 2018-12-30.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc32-c"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc30-c"

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

    srand(static_cast<unsigned int>(time(nullptr)));
    Size S[] = {SPARK_GC_HEAP_SMALL,
                SPARK_GC_HEAP_SMALL,
                SPARK_GC_HEAP_SMALL,
                SPARK_GC_HEAP_SMALL,
                SPARK_GC_HEAP_MEDIUM,
                SPARK_GC_HEAP_MEDIUM};
    for (int i = 0; i < 50; ++i) {
        Size s = S[static_cast<Size>(rand() % (sizeof(S) / sizeof(S[0])))];
        heap->allocate(s);
    }

    heap->dumpHeap(stdout);
    delete heap;
    free(memory);
    return EXIT_SUCCESS;
}
