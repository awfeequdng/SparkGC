//
// Created by kiva on 2018-12-30.
//
#pragma once

namespace spark {
    /**
     * Four bits in this bitmap for every eight bytes of heap memory,
     * with very fast mapping from an object to its color and vice versa.
     * Sweep scans this bitmap without touching the objects.
     */
    class ColorBitmap {

    };
}
