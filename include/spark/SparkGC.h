//
// Created by kiva on 2018-12-30.
//
#pragma once

#include <spark/SparkGC_Shared.h>
#include <spark/CollectedHeap.h>
#include <spark/CollectedObject.h>
#include <spark/ColorBitmap.h>
#include <vector>
#include <set>

namespace spark {
    class SparkMutator {
    public:
        SparkGC *sparkGC;
        GCHandshakeState handshakeState;
        Addr lastRead;
        Addr lastWrite;
        GCColor allocationColor;
        std::vector<CollectedObject *> markBuffer;

    private:
        void update(Addr obj, Addr newObj);
    };

    class SparkGC {
    private:
        GCHandshakeState handshakeState;
        GCStage gcStage;
        GCColor clearColor;
        GCColor markColor;
        ColorBitmap heapColors;
        CollectedHeap *heap;
        std::set<SparkMutator *> mutators;

    private:
        explicit SparkGC(CollectedHeap *heap);

        void stageClear();

        void stageMark();

        void stageTrace();

        void stageSweep();

        void stageDone();

        void markGlobalRoot();

        void collectorTrace();

        void processWeakRefs();

        void collectorSweep();

        void toggleColor();

        void handshakeMutators(GCHandshakeState state);

        void waitHandshake();

        void postHandshake(GCHandshakeState state);

        void postState(GCStage stage);

    public:
        SparkGC(const SparkGC &) = delete;

        SparkGC(SparkGC &&) = delete;

        ~SparkGC();

        void registerMutator(SparkMutator *mutator);

        void unregisterMutator(SparkMutator *mutator);

        void collect();
    };
}
