//
// Created by kiva on 2018-12-28.
//
#pragma once

#include <cstdlib>

namespace spark {
    using Offset = size_t;

    /**
     * A color abstraction is used to indicate the state of
     * an object with respect to collection.
     * Colors are assigned to objects or memory locations.
     */
    enum GCColor {
        /**
         * Memory that is unallocated.
         */
        BLUE,

        /**
         * Memory that has not been scanned by the collector
         * during the current GC cycle. Memory left white after
         * the collector has completed tracing is garbage.
         */
        WHITE,

        /**
         * Objects marked by the collector, but
         * whose children may have not been marked.
         */
        GRAY,

        /**
         * Objects marked by the collector,
         * whose direct children have also been marked.
         */
        BLACK,
    };

    /**
     * Each of the mutator threads and the collector thread have its own status
     * variable. Mutator threads acknowledge a change in the collector’s status
     * by changing their own status to the corresponding status,
     * which is called a handshake.
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
    enum GCState {
        GC_SYNC1,
        GC_SYNC2,
        GC_ASYNC,
    };
}
