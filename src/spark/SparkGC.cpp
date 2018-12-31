//
// Created by kiva on 2018-12-31.
//
#include <spark/SparkGC.h>

namespace spark {

    SparkGC::SparkGC(CollectedHeap *heap)
        : heap(heap), handshakeState(GC_ASYNC), gcStage(GC_RESTING),
          clearColor(GC_COLOR_WHITE), markColor(GC_COLOR_BLACK) {
        Size max = heap->getMaxBlockCount() + 1;
        for (int i = 0; i < max; ++i) {
            heapColors.createBitmap();
        }
    }

    SparkGC::~SparkGC() = default;

    void SparkGC::collect() {
        stageClear();
        stageMark();
        stageTrace();
        stageSweep();
        stageDone();
    }

    void SparkGC::markGlobalRoot() {

    }

    void SparkGC::processWeakRefs() {

    }

    void SparkGC::collectorTrace() {

    }

    void SparkGC::collectorSweep() {

    }

    void SparkGC::stageClear() {
        postState(GC_CLEAR_OR_MARKING);
        handshakeMutators(GC_SYNC1);
    }

    void SparkGC::stageMark() {
        handshakeMutators(GC_SYNC2);
        postState(GC_TRACING);
        postHandshake(GC_ASYNC);
        markGlobalRoot();
        waitHandshake();
    }

    void SparkGC::stageTrace() {
        collectorTrace();
        postState(GC_REF_PROCESSING);
        processWeakRefs();
    }

    void SparkGC::stageSweep() {
        postState(GC_SWEEPING);
        collectorSweep();
    }

    void SparkGC::stageDone() {
        // Change the interpretation of the colors
        // for the next gc cycle.
        toggleColor();
        postState(GC_RESTING);
    }

    /**
     * According to the basic mark-sweep algorithm, the color of
     * all black (marked) objects must be reset to white (clear)
     * in preparation for the next GC cycle. We introduce a color toggle
     * instead of changing the color for each black object.
     * The color toggle simply changes the interpretation of the colors.
     * There are three main advantages to this modification:
     *
     * 1) it reduces the amount of work that the collector must do,
     *    since the collector no longer is required to reset black
     *    items to white in sweep.
     * 2) it simplifies and speeds up object creation by removing
     *    the checks for the location of the sweep pointer.
     * 3) it removes the the store-load dependency in object creation.
     */
    void SparkGC::toggleColor() {
        std::swap(clearColor, markColor);
    }

    void SparkGC::handshakeMutators(GCHandshakeState state) {
        postHandshake(state);
        waitHandshake();
    }

    void SparkGC::waitHandshake() {
        Size mutatorCount = mutators.size();
        Size readyCount = 0;
        do {
            for (auto m : mutators) {
                if (m->handshakeState == handshakeState) {
                    ++readyCount;
                }
            }
        } while (readyCount != mutatorCount);
    }

    void SparkGC::postHandshake(GCHandshakeState state) {
        handshakeState = state;
    }

    void SparkGC::postState(GCStage stage) {
        gcStage = stage;
    }

    void SparkGC::registerMutator(SparkMutator *mutator) {
        if (mutator == nullptr) {
            throw std::exception();
        }
    }

    void SparkGC::unregisterMutator(SparkMutator *mutator) {
    }
}
