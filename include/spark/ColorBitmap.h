//
// Created by kiva on 2018-12-30.
//
#pragma once

#include <spark/SparkGC_Shared.h>
#include <bitset>
#include <vector>

#define SPARK_GC_BLOCK_BITMAP ((SPARK_GC_HEAP_BLOCK) / 4 * 2)

namespace spark {
    /**
     * Two bits in this bitmap for every four bytes of heap memory,
     * with very fast mapping from an object to its color and vice versa.
     * Sweep scans this bitmap without touching the objects.
     */
    class ColorBitmap {
    public:
        template<typename T>
        using Vector = std::vector<T>;

        template<Size N>
        using Bitset = std::bitset<N>;

        using Bitmap = Bitset<SPARK_GC_BLOCK_BITMAP>;

    private:
        Vector<Bitmap> bitmaps;

    public:
        void createBitmap() {
            bitmaps.push_back(Bitmap());
        }

        Bitmap &getBitmap(Size offset) {
            Size blockIndex = offset / SPARK_GC_HEAP_BLOCK;
            return bitmaps.at(blockIndex);
        }

        const Bitmap &getBitmap(Size offset) const {
            Size blockIndex = offset / SPARK_GC_HEAP_BLOCK;
            return bitmaps.at(blockIndex);
        }

        GCColor getColor(Size offset) const {
            auto &bitmap = getBitmap(offset);
            Size bitmapOffset = (offset % SPARK_GC_HEAP_BLOCK) / 4 * 2;
            Bitset<2> colorBit;
            for (int i = 0; i < 2; ++i) {
                colorBit.set(Size(i), bitmap.test(bitmapOffset + i));
            }
            return static_cast<GCColor>(colorBit.to_ulong());
        }

        void setColor(Size offset, GCColor color) {
            auto &bitmap = getBitmap(offset);
            Size bitmapOffset = (offset % SPARK_GC_HEAP_BLOCK) / 4 * 2;
            Bitset<2> colorBit(color);
            for (int i = 0; i < 2; ++i) {
                bitmap.set(bitmapOffset + i, colorBit.test(Size(i)));
            }
        }
    };
}
