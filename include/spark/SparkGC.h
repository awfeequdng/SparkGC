//
// Created by kiva on 2018-12-30.
//
#pragma once

#include <spark/SparkGC_Shared.h>
#include <spark/CollectedHeap.h>
#include <spark/CollectedObject.h>
#include <spark/ColorBitmap.h>
#include <set>
#include <stack>

namespace spark {
    class SparkGC {
    private:
        GCHandshakeState handshakeState;
        GCStage gcStage;
        GCColor clearColor;
        GCColor markColor;
        ColorBitmap heapColors;
        CollectedHeap *heap;
        std::set<SparkMutator *> mutators;
        std::stack<Addr> markBuffer;
        std::stack<Addr> weakRefs;

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

        void emptyMarkBuffer();

        void collectorMarkGray(Addr addr);

        void collectorMarkBlack(Addr addr);

        bool queryIsWeakReference(Addr addr);

        void toggleColor();

        void handshakeMutators(GCHandshakeState state);

        void waitHandshake() const;

        void postHandshake(GCHandshakeState state);

        void postState(GCStage stage);

        Size offsetOf(Addr addr) const;

    public:

        SparkGC(const SparkGC &) = delete;

        SparkGC(SparkGC &&) = delete;

        ~SparkGC();

        GCStage getStage() const noexcept {
            return gcStage;
        }

        void registerMutator(SparkMutator *mutator);

        void unregisterMutator(SparkMutator *mutator);

        void collect();

        Addr allocate(Size size);

        /**
         * For mutators.
         * @param addr
         */
        void markGray(SparkMutator *m, Addr addr);

        void setColor(Addr addr, GCColor color);

        GCHandshakeState getHandshakeState() const noexcept {
            return handshakeState;
        }

        GCColor getMarkColor() const noexcept {
            return markColor;
        }
    };
}
