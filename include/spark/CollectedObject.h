//
// Created by kiva on 2018-12-30.
//
#pragma once

#include <spark/SparkGC_Shared.h>

namespace spark {
    class ColorMarker {
        friend class SparkGC;

    private:
        SparkGC *sparkGC;

        explicit ColorMarker(SparkGC *sparkGC);

    public:
        void mark(CollectedObject *object);
    };

    class CollectedObject {
    public:
        virtual Size getOnStackSize() = 0;

        virtual void markChildren(ColorMarker &marker) = 0;

        virtual ~CollectedObject() = default;
    };
}
