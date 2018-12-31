//
// Created by kiva on 2018-12-31.
//

#include <spark/CollectedObject.h>
#include <spark/SparkGC.h>

namespace spark {
    ColorMarker::ColorMarker(SparkGC *sparkGC)
        : sparkGC(sparkGC) {
    }

    void ColorMarker::mark(CollectedObject *object) {
        sparkGC->collectorMarkGray(Addr(object));
    }
}
