//
// Created by kiva on 2018-12-28.
//
#pragma once

#include <cstdio>

namespace spark {
    class Object;

    struct ObjectHeader {
        Object *forwarding;
        int tag;
    };

    class Object {
    private:
        ObjectHeader objectHeader;
        size_t objectSize;

    public:
        size_t getSize() noexcept {
            return objectSize;
        }
    };
}
