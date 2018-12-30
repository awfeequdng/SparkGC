//
// Created by kiva on 2018-12-30.
//
#pragma once

#include <spark/SparkGC_Shared.h>

namespace spark {
    class CollectedObject {
    public:
        virtual Size getOnStackSize() = 0;
    };
}
