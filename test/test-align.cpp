//
// Created by kiva on 2019-01-01.
//
#include <cstdio>
#include <spark/SparkGC_Shared.h>
#include <spark/CollectedHeap.h>
#include <spark/CollectedObject.h>

#define A(size, expected) \
    if (spark::CollectedHeap::align(size) != (expected)) {          \
        fprintf(stderr, "%zd is not aligned to %zd but %zd\n",      \
            spark::Size(size), spark::Size(expected),               \
            spark::CollectedHeap::align(size));                     \
        abort();                                                    \
    } else {                                                        \
        fprintf(stderr, "%zd is aligned to %zd\n",                  \
            spark::Size(size), spark::Size(expected));              \
    }

int main() {
    A(sizeof(int), SPARK_GC_HEAP_SMALL);
    A(sizeof(double), SPARK_GC_HEAP_SMALL);
    A(sizeof(spark::ColorMarker), SPARK_GC_HEAP_SMALL);
    A(sizeof(char[32]), SPARK_GC_HEAP_SMALL);
    A(sizeof(char[33]), SPARK_GC_HEAP_MEDIUM);
    A(sizeof(char[35]), SPARK_GC_HEAP_MEDIUM);
    A(sizeof(char[37]), SPARK_GC_HEAP_MEDIUM);
    A(sizeof(char[48]), SPARK_GC_HEAP_MEDIUM);
    A(sizeof(char[60]), SPARK_GC_HEAP_MEDIUM);
    A(sizeof(char[63]), SPARK_GC_HEAP_MEDIUM);
    A(sizeof(char[64]), SPARK_GC_HEAP_MEDIUM);
}
