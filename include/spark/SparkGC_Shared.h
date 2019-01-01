//
// Created by kiva on 2018-12-28.
//
#pragma once

#include <cstdlib>

/**
 * sizeof(intOopDesc)
 */
#define SPARK_GC_HEAP_SMALL 32

/**
 * sizeof(instanceOopDesc)
 */
#define SPARK_GC_HEAP_MEDIUM 64

#define SPARK_GC_HEAP_BLOCK 128

#define SPARK_GC_ALIGN SPARK_GC_HEAP_SMALL

#define SPARK_GC_SWEEP_THREADS 4

namespace spark {
    using Offset = size_t;

    using Size = size_t;

    using Addr = unsigned char *;

    class SparkMutator;
    class SparkGC;
    class CollectedObject;
    class CollectedHeap;
    class ColorMarker;

    /**
     * A color abstraction is used to indicate the state of
     * an object with respect to collection.
     * Colors are assigned to objects or memory locations.
     *
     * Note that: meaning of white and black color may change
     * after a gc cycle due to the Color Toggle.
     */
    enum GCColor {
        /**
         * Memory that is unallocated.
         */
            GC_COLOR_BLUE,

        /**
         * Memory that has not been scanned by the collector
         * during the current GC cycle. Memory left white after
         * the collector has completed tracing is garbage.
         */
            GC_COLOR_WHITE,

        /**
         * Objects marked by the collector, but
         * whose children may have not been marked.
         */
            GC_COLOR_GRAY,

        /**
         * Objects marked by the collector,
         * whose direct children have also been marked.
         */
            GC_COLOR_BLACK,
    };

    /**
     * Each of the mutator threads and the collector thread have its own status
     * variable. Mutator threads acknowledge a change in the collector’s status
     * by changing their own status to that of the collector，
     * which is called a handshake.
     * Handshake is used to avoid expensive synchronizations.
     * Note that: A mutator may not handshake while it is updating an object slot
     * or creating an object.
     *
     * At the start of the GC cycle, the collector sets its status to {@code GC_SYNC1}.
     * During sync1, mutators create objects white.
     * When a slot containing a pointer to an object is updated,
     * the write barrier shades gray both the object referenced by the old value
     * and the object referenced by the new value.
     *
     * After all mutators have moved into sync1, the collector sets its status
     * to {@code GC_SYNC2} and requires the mutators to handshake. The write barrier action
     * and the color chosen for object creation during sync2 remain the same as for sync1.
     * The sync2 acts as a tripwire to guarantee that all memory transactions
     * that started before all mutators transitioned to sync1 have completed.
     *
     * Once all threads are in sync2, the collector moves to {@code GC_ASYNC}.
     * The trace and sweep occur during async. The collector remains in async
     * until it begins the next collection cycle.
     *
     * As each mutator responds to async, it marks its roots for the collector to trace.
     * Once a mutator has marked its local roots, it is only required to gray the object
     * referenced by the old value on update. This graying of the old reference by
     * the write barrier continues until the collector completes the trace.
     *
     * At the beginning of async, a mutator also begins to color its newly created objects black.
     * This continues until sweep. During sweep the location of the collector’s sweep pointer
     * determines the color for newly created objects:
     * white if it is created in memory which has already been swept,
     * black if not yet swept,
     * and gray if it is created at the point where the collector is sweeping.
     * Starting with the completion of sweep all objects are created white.
     */
    enum GCHandshakeState {
        GC_SYNC1,
        GC_SYNC2,
        GC_ASYNC,
    };


    enum GCStage {
        GC_CLEAR_OR_MARKING,
        GC_TRACING,
        GC_REF_PROCESSING,
        GC_SWEEPING,
        GC_RESTING
    };
}
