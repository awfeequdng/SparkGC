//
// Created by kiva on 2018-12-31.
//
#pragma once

#include <spark/SparkGC.h>
#include <spark/CollectedObject.h>
#include <spark/SparkGC_Shared.h>
#include <vector>

namespace spark {
    class SparkMutator {
    public:
        SparkGC *sparkGC;
        GCHandshakeState handshakeState;
        Size lastWrite;
        Size lastRead;
        GCColor allocationColor;
        std::vector<Addr> markBuffer;

        SparkMutator();

    private:
        void markRootElements();

    protected:

        /**
         * The write barrier during gc.
         * Place an {@code obj} on {@code address}.
         * @param addr
         * @param obj
         */
        void write(CollectedObject *addr, CollectedObject *obj);

        /**
         * The allocate barrier.
         * @param size Size to allocate
         * @return address
         */
        Addr allocate(Size size);

        void handshakeCollector();
    };
}
