//
// Created by kiva on 2018-12-31.
//
#include <spark/SparkMutator.h>

namespace spark {
    SparkMutator::SparkMutator()
        : lastWrite(0), lastRead(0), sparkGC(nullptr),
          handshakeState(GC_ASYNC), allocationColor(GC_COLOR_WHITE) {
    }

    Addr SparkMutator::allocate(Size size) {
        Addr addr = sparkGC->allocate(size);
        sparkGC->setColor(addr, allocationColor);
        return addr;
    }

    void SparkMutator::write(CollectedObject *addr, CollectedObject *obj) {
        if (handshakeState != GC_ASYNC) {
            sparkGC->markGray(this, (Addr) addr);
            sparkGC->markGray(this, (Addr) obj);
        } else if (sparkGC->getStage() == GC_TRACING) {
            sparkGC->markGray(this, (Addr) addr);
        }
    }

    void SparkMutator::handshakeCollector() {
        auto state = sparkGC->getHandshakeState();
        if (handshakeState != state) {
            if (handshakeState == GC_SYNC2) {
                markRootElements();
                allocationColor = sparkGC->getMarkColor();
            }
            handshakeState = state;
        }
    }

    void SparkMutator::markRootElements() {
        // TODO
    }
}

