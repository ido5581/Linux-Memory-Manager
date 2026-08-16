# Linux Memory Manager

A custom dynamic memory allocator implemented in **C** for Linux.

The project started as a course-based implementation of a memory allocator and was then extended and modified to explore low-level memory management concepts, including custom free-block management, virtual memory allocation, block splitting/coalescing, and `realloc`.

## Features

* Custom `malloc` implementation
* Custom `free` implementation
* Custom `calloc` implementation
* Custom `realloc` implementation
* Virtual memory allocation using `mmap`
* Virtual memory release using `munmap`
* Block splitting
* Adjacent free-block coalescing
* Custom metadata for allocated and free blocks
* **Intrusive Max Heap** for managing free blocks
* Multiple page families for different allocation sizes

## Architecture

The allocator manages memory using a hierarchy of page families and memory blocks.

Each memory block contains metadata describing the block and, when the block is free, the metadata also contains the links required by the intrusive heap.

A simplified block layout is:

```text
+---------------------------+
| Block Metadata            |
|                           |
| block_size                |
| is_free                   |
| prev_block / next_block   |
|                           |
| Intrusive Heap Node       |
| parent / left / right     |
+---------------------------+
| User Data                 |
+---------------------------+
```

Free blocks are managed using an **Intrusive Max Heap**, allowing the allocator to efficiently locate large available blocks without allocating separate heap-node objects.

A simplified view of the allocator is:

```text
                 Memory Manager
                       |
                +------+------+
                |             |
          Page Families   Free Blocks
                |             |
          Virtual Pages   Intrusive
                         Max Heap
                |
        +-------+-------+
        |       |       |
      Block   Block   Block
```

## Intrusive Max Heap

The free-block data structure is implemented as an **Intrusive Max Heap**.

Instead of allocating a separate heap node for every free memory block, the heap-related pointers are embedded directly inside the block metadata.

This means that a free memory block simultaneously acts as:

1. A memory-management block.
2. A node in the free-block heap.

### Why an Intrusive Data Structure?

Using an intrusive data structure avoids allocating additional objects for heap nodes.

This provides several advantages:

* No separate allocation for heap nodes
* Lower memory-management overhead
* Fewer pointer indirections
* Better memory locality
* Direct integration between block metadata and the heap
* Reuse of existing memory-management metadata

The heap is implemented as a **Max Heap**, allowing efficient access to the largest available free block.

## Allocation Strategy

When an allocation request is made, the allocator searches the free-block Max Heap for a suitable block.

If a free block is larger than the requested allocation, the block can be split.

```text
Before:

+------------------------------------------+
|              Free Block                  |
+------------------------------------------+

After:

+-------------------+----------------------+
|   Allocated       |       Free           |
|      Block        |       Block          |
+-------------------+----------------------+
```

The remaining free block is inserted back into the Intrusive Max Heap.

If no suitable free block exists, the allocator obtains additional virtual memory from the operating system.

## Free and Coalescing

When a block is released, it is marked as free and its neighboring blocks are examined.

Adjacent free blocks can be merged to reduce fragmentation.

```text
Before:

+-----------+-----------+-----------+
|   Free    |   Free    | Allocated |
+-----------+-----------+-----------+

After:

+-----------------------+-----------+
|      Free Block       | Allocated |
+-----------------------+-----------+
```

When blocks are merged, the corresponding heap state is updated so that the resulting free block is correctly represented in the Intrusive Max Heap.

## `realloc`

The allocator implements several `realloc` paths.

### Same Size

If the requested size is equal to the current allocation size, the existing pointer can be returned.

### Shrinking

If the requested size is smaller than the current block, the allocator attempts to split the unused portion into a new free block.

```text
Before:

+--------------------------------+
|          Allocated             |
+--------------------------------+

After:

+----------------+---------------+
|   Allocated    |     Free      |
+----------------+---------------+
```

### Growing In-Place

If the block immediately following the current allocation is free and provides enough space, the allocator can expand the current block without moving the existing data.

```text
Before:

+-------------+-------------------+
| Allocated A |      Free B       |
+-------------+-------------------+

After:

+-----------------------+---------+
|      Allocated A      |  ...    |
+-----------------------+---------+
```

The free block is removed from the Intrusive Max Heap before being merged with the current allocation.

### Relocation

If the block cannot be expanded in place, a new allocation is created, the existing data is copied, and the old allocation is released.

```text
Old:

+----------------+
| Existing Data  |
+----------------+

        ↓

New:

+------------------------+
| Existing Data + Space  |
+------------------------+
```

## Virtual Memory

The allocator obtains virtual memory from Linux using `mmap` and releases unused memory regions using `munmap`.

This allows the allocator to interact directly with the operating system's virtual memory interface rather than relying on the standard C allocator.

## Page Families

The allocator organizes memory into **page families** based on allocation size.

Each page family is associated with a specific block size, allowing allocations of similar sizes to be grouped together.

This provides a structured way to manage pages and their blocks while keeping track of the relationship between:

```text
Page Family
     |
     +--- Virtual Page
     |       |
     |       +--- Block
     |       +--- Block
     |       +--- Block
     |
     +--- Virtual Page
             |
             +--- Block
             +--- Block
```

## Project Structure

```text
.
├── mm.c
├── mm.h
├── uapi_mm.h
├── glHeap.c
├── glHeap.h
├── test.c
└── README.md
```

### `mm.c`

Core memory allocator implementation, including:

* Allocation
* Deallocation
* Block splitting
* Block coalescing
* `calloc`
* `realloc`
* Virtual memory management
* Page-family management

### `glHeap.c / glHeap.h`

Implementation of the custom **Intrusive Max Heap** used to manage free blocks.

### `test.c`

Tests and examples used to exercise the allocator.

### `mm.h / uapi_mm.h`

Allocator interfaces and public definitions.

## Key Low-Level Concepts

This project focuses on practical use of:

* C pointers
* Pointer arithmetic
* Structs and metadata
* Memory layout
* Dynamic memory management
* Virtual memory
* `mmap` / `munmap`
* Intrusive data structures
* Binary heaps
* Max Heaps
* Linked structures
* Memory fragmentation
* Block splitting
* Block coalescing
* `memcpy`
* `memset`

## Design Decisions

### Intrusive Max Heap

Free blocks are represented directly inside the allocator's metadata and participate in the Max Heap without requiring separate heap-node allocations.

This reduces overhead and demonstrates how intrusive data structures can be used in low-level systems programming.

### Max Heap

A Max Heap was chosen to efficiently retrieve the largest available free block.

This provides an efficient way to search for large free blocks while keeping the heap integrated with the allocator's metadata.

### `mmap` / `munmap`

The allocator uses Linux virtual memory APIs to obtain and release memory regions.

This provides direct interaction with Linux's virtual memory subsystem.

## Limitations

This is an educational memory allocator and is **not intended to replace the system allocator**.

Some production-level features are outside the scope of the project, including:

* Thread safety
* Advanced fragmentation mitigation
* Production-grade error handling
* Full compatibility with the system `malloc` ABI
* Extensive performance optimization

## Learning Goals

The main goal of the project was to gain practical experience with low-level memory management in C and understand what happens underneath a high-level allocation such as:

```c
void *ptr = malloc(size);
```

The project explores how an allocator can:

1. Request virtual memory from the operating system.
2. Organize memory into page families.
3. Divide memory into manageable blocks.
4. Track metadata for each block.
5. Manage free blocks using an intrusive data structure.
6. Find suitable free blocks using a Max Heap.
7. Split oversized blocks.
8. Merge adjacent free blocks.
9. Resize allocations using `realloc`.
10. Return unused virtual memory to the operating system.

## Acknowledgements

The initial implementation was developed as part of a memory-management course project.

The project was subsequently extended and modified to experiment with different design choices, including:

* An **Intrusive Max Heap** for free-block management
* Linux `mmap` / `munmap` based virtual memory management
* A custom `calloc` implementation
* A custom `realloc` implementation
* Block splitting and coalescing
* Custom memory metadata and page-family management
