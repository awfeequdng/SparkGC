## Heap Management

### Overview
Our heap manager uses a two-level allocation scheme. We divide objects into 4 types: small, medium, large and super, using a different allocation scheme for each. As allocation of small and medium objects is a frequent operation, and therefore we designed small and medium object allocation for efficiency. Large and super object allocation is less frequent, and also slower due to block increments.

### Small, Medium, Large and Super Objects
We divide memory into blocks (in SparkGC's implementation we choose the block size to be `SPARK_GC_HEAP_BLOCK`, namely 128 bytes). An object is small if its size is between `0` and `SPARK_GC_HEAP_SMALL`, medium if its size is between `SPARK_GC_HEAP_SMALL` and `SPARK_GC_HEAP_MEDIUM`, large if its size is greater than one third of a block, and super if its size is greater than that of a block (namely `SPARK_GC_HEAP_BLOCK`). 

### Allocation Strategy
Ranges of free memory blocks are held in a list sorted first by size and then by address. We call these blocks `HeapBlock`s (also `pre-sized chunk`s).

Small and medium object allocation is done from bins of pre-sized chunks. Large and super object allocations are done in block increments, i.e., large objects are rounded up in size to full block increments.

When a request for a large object is received, the heap will select a range of free blocks on a best-fit basis. If there is more than one fit, it selects the one starting at the lowest address. If this range is larger than the requested large object (remembering that large objects are allocated on block boundaries), then it will be split to return an object of the requested size, and a remainder. The remainder will be reinserted into the free block list. Initially the block list consists of only a single block. We will protect the block list using a mutual exclusion lock, which is acquired for the duration of an individual operation on the list.

### The Color Bitmap
The heap maintains a color bitmap, two bits for every four bytes of heap memory, with very fast mapping from an object to its color and vice versa. 
Sweep scans this bitmap without touching the objects. We arranged the mapping so that the color for two consecutive objects is never in the same byte of bitmap, so that accessing of the color can occur without synchronization. 
The color bitmap also enables fast conservative pointer detection while scanning the heaps.
