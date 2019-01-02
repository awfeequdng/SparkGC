//
// Created by kiva on 2018-12-31.
//
#include <spark/SparkMutator.h>

namespace spark {
    SparkMutator::SparkMutator()
        : lastWrite(0), lastRead(0), sparkGC(nullptr),
          handshakeState(GC_ASYNC), allocationColor(GC_COLOR_WHITE) {
    }

    SparkMutator::~SparkMutator() = default;

    Addr SparkMutator::allocate(Size size) {
        Addr addr = sparkGC->allocate(size);
        sparkGC->setColor(addr, allocationColor);
        return addr;
    }

    void SparkMutator::write(CollectedObject *addr, CollectedObject *obj) {
        if (handshakeState != GC_ASYNC) {
            markGray(addr);
            markGray(obj);
        } else if (sparkGC->getStage() == GC_TRACING) {
            markGray(addr);
        }
    }

    void SparkMutator::handshakeCollector() {
        auto state = sparkGC->getHandshakeState();
        if (handshakeState != state) {
            if (handshakeState == GC_SYNC2) {
                markGlobalRoot();
                allocationColor = sparkGC->getMarkColor();
            }
            handshakeState = state;
        }
    }

    void SparkMutator::markGray(CollectedObject *object) {
        sparkGC->markGray(this, Addr(object));
    }
}

