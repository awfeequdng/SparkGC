//
// Created by kiva on 2018-12-30.
//
#pragma once

#include <spark/SparkGC_Shared.h>
#include <spark/CollectedHeap.h>
#include <spark/CollectedObject.h>
#include <array>
#include <vector>

namespace spark {
    class SparkMutator {
    private:
        SparkGC *sparkGC;
        GCHandshakeState handshakeState;
        Addr lastRead;
        Addr lastWrite;
        std::vector<CollectedObject *> markBuffer;

    public:
    };

    class SparkGC {
    private:
        GCHandshakeState handshakeState;
        GCStage gcStage;
        GCColor allocationColor;
        GCColor markColor;
        CollectedHeap *heap;
        std::array<SparkMutator, SPARK_GC_MUTATOR_COUNT> mutators;

    private:
        SparkGC(CollectedHeap *heap);

        void stageClear();

        void stageMark();

        void stageTrace();

        void stageSweep();

    public:
        SparkGC(const SparkGC &) = delete;

        SparkGC(SparkGC &&) = delete;

        ~SparkGC();
    };
}
