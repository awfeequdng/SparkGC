//
// Created by kiva on 2018-12-28.
//
#pragma once

#include <cstdio>

namespace spark {
    class Object;

    struct ObjectHeader {
        enum {
            TAG_NONE, TAG_MARKED
        };

        Object *forwarding = nullptr;
        int tag = TAG_NONE;

        bool isTagged() const noexcept {
            return tag == TAG_MARKED;
        }

        void setTagged(bool tagged) {
            tag = tagged ? TAG_MARKED : TAG_NONE;
        }

        bool operator==(ObjectHeader &&other) const noexcept {
            return other.forwarding == this->forwarding
                   && other.tag == this->tag;
        }

        bool operator==(const ObjectHeader &other) const noexcept {
            return other.forwarding == this->forwarding
                   && other.tag == this->tag;
        }
    };

    class Object {
        friend class Mutator;

    private:
        ObjectHeader objectHeader;
        size_t objectSize = 0;

    public:
        size_t getSize() noexcept {
            return objectSize;
        }
    };
}
