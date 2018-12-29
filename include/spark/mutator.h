//
// Created by kiva on 2018-12-28.
//
#pragma once

#include <compileTimeConfig.h>

#if SPARK_ARCH_arm

#include <spark/arch/arm/atomic.h>

#elif SPARK_ARCH_aarch64

#include <spark/arch/aarch64/atomic.h>

#elif SPARK_ARCH_x86

#include <spark/arch/x86/atomic.h>

#elif SPARK_ARCH_x86_64

#include <spark/arch/x86_64/atomic.h>

#endif

#include <spark/gc_shared.h>

namespace spark {
    class Mutator {
    private:
        template<typename T>
        static bool compareAndSwap(T *ptr, T old, T new_) {
            return cmpxchg(ptr, old, new_) == new_;
        }

        static bool compareAndSwapObject(Object **ptr, Object *old, Object *new_) {
            // TODO:
            //  redefine equality for `Object* a == Object* b`
            //  that it returns true even if `b` is a forwarding pointer to a
            //  (or vise-versa).
            return compareAndSwap(ptr, old, new_);
        }

    public:
        /**
         * The Brooks-styled read barrier.
         * @tparam T filed type
         * @param o object
         * @param offset filed offset
         * @return field value
         */
        template<typename T>
        static T read(Object *o, Offset offset) {
            return *(o->objectHeader.forwarding + offset);
        }

        /**
         * The read barrier specialized for object references,
         * in which both the object being loaded from
         * as well as the value loaded are forwarded.
         * @param o object itself
         * @param offset field offset
         * @return field object
         */
        static Object *read(Object *o, Offset offset) {
            auto result = o->objectHeader.forwarding + offset;
            if (result != nullptr) {
                result = result->objectHeader.forwarding;
            }
            return result;
        }

        template<typename T>
        static void write(Object *o, Offset offset, T value) {
            updateHeader(o, o->objectHeader.forwarding);
            *(o->objectHeader.forwarding + offset) = value;
        }

        static void updateHeader(Object *o, Object *forwarding) {
            if (o->objectHeader.isTagged()) {
                ObjectHeader newHeader;
                newHeader.forwarding = forwarding;
                newHeader.setTagged(false);

                compareAndSwap(&o->objectHeader,
                    o->objectHeader,
                    newHeader);
            }
        }
    };
}
