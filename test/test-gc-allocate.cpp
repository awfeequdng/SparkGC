//
// Created by kiva on 2019-01-01.
//
#include <spark/SparkGC.h>

using namespace spark;

int main() {
    constexpr Size MEM_128M = 128 * 1024 * 1024;
    SparkGC* gc = SparkGC::newGC(MEM_128M);

    gc->collect();
    SparkGC::deleteGC(gc);
}
