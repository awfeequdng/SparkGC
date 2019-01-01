//
// Created by kiva on 2018-12-30.
//
#include <spark/CollectedHeap.h>
#include <spark/CollectedObject.h>
#include <spark/ColorBitmap.h>
#include <cstdlib>
#include <cstdio>

class MyObject : public spark::CollectedObject {
private:
    spark::Size size;
    int id;

public:
    explicit MyObject(spark::Size size, int id) : size(size), id(id) {}

    spark::Size getOnStackSize() override {
        return size;
    }

    void markChildren(spark::ColorMarker &marker) override {
    }

    int getId() const {
        return id;
    }
};

const char *colorToString(spark::GCColor color) {
    switch (color) {
        case spark::GC_COLOR_BLUE:
            return "blue";
        case spark::GC_COLOR_WHITE:
            return "white";
        case spark::GC_COLOR_GRAY:
            return "gray";
        case spark::GC_COLOR_BLACK:
            return "black";
        default:
            return "wtf";
    }
}

void colorBitmapSize(spark::Size heapSize) {
    printf("HeapSize = %zd bytes, ColorBitmap = %zd bytes\n",
        heapSize,
        sizeof(spark::ColorBitmap) +
        sizeof(spark::ColorBitmap::Bitmap) * (heapSize / SPARK_GC_HEAP_BLOCK));
}

int main() {
    using namespace spark;

    GCColor blue = GC_COLOR_BLUE;
    GCColor white = GC_COLOR_WHITE;
    GCColor gray = GC_COLOR_GRAY;
    GCColor black = GC_COLOR_BLACK;

    ColorBitmap::Bitset<2> BLUE(blue);
    ColorBitmap::Bitset<2> WHITE(white);
    ColorBitmap::Bitset<2> GRAY(gray);
    ColorBitmap::Bitset<2> BLACK(black);

    Size S[] = {SPARK_GC_HEAP_SMALL,
                SPARK_GC_HEAP_SMALL,
                SPARK_GC_HEAP_SMALL,
                SPARK_GC_HEAP_SMALL,
                SPARK_GC_HEAP_MEDIUM,
                SPARK_GC_HEAP_MEDIUM,
                SPARK_GC_HEAP_MEDIUM,
                SPARK_GC_HEAP_MEDIUM};

    Size memorySize4K = 4096;
    Addr memory = (Addr) malloc(memorySize4K);
    if (memory == nullptr) {
        return EXIT_FAILURE;
    }

    ColorBitmap colorBitmap;
    for (int i = 0; i < (memorySize4K / SPARK_GC_HEAP_BLOCK); ++i) {
        colorBitmap.createBitmap();
    }

    colorBitmapSize(memorySize4K);
    colorBitmapSize(1024 * 1024 * 1024);

    auto heap = new CollectedHeap(memory, memorySize4K);

    srand(static_cast<unsigned int>(time(nullptr)));

    for (int j = 0; j < 50; ++j) {
        Size s = S[static_cast<Size>(rand() % (sizeof(S) / sizeof(S[0])))];
        Addr addr = heap->allocate(s);
        Size offset = addr - heap->getHeapStart();

        (void) new(addr) MyObject(s, j);
        colorBitmap.setColor(offset, white);
    }

    // Scan the whole heap
    Addr current = heap->getHeapStart();
    while (current < heap->getHeapEnd()) {
        Size offset = current - heap->getHeapStart();
        GCColor color = colorBitmap.getColor(offset);
        printf("at %zd, color is %s\n", offset, colorToString(color));
        if (color != blue) {
            auto obj = (MyObject *) current;
            printf("\tcolor is not blue, here is an object\n");
            printf("\tobject size = %zd, id = %d\n",
                obj->getOnStackSize(), obj->getId());
        }

        // move forward
        current += SPARK_GC_ALIGN;
    }

    delete heap;
    free(memory);
    return EXIT_SUCCESS;
}

