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
* Custom metadata for allocated/free blocks
* Max Heap for managing free blocks
* Multiple page families for different allocation sizes

## Architecture

The allocator manages memory using a hierarchy of page families and memory blocks.

Each allocated region contains metadata describing the block:

```text
+-------------------+
| Block Metadata    |
+-------------------+
| User Data         |
+-------------------+
```

Free blocks are maintained in a **Max Heap**, allowing the allocator to efficiently locate a large free block for a new allocation.

A simplified view of the allocator is:

```text
                 Memory Manager
                       |
                +------+------+
                |             |
          Page Families   Free Blocks
                |             |
          Virtual Pages    Max Heap
                |
        +-------+-------+
        |       |       |
      Block   Block   Block
```

## Allocation Strategy

When a request is made, the allocator attempts to find a suitable free block.

If a free block is larger than the requested allocation, the block can be split:

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

The remaining free block is inserted back into the Max Heap.

## Free and Coalescing

When a block is released, it is marked as free and neighboring free blocks are checked.

Adjacent free blocks can be merged to reduce fragmentation:

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

## `realloc`

The allocator implements several `realloc` paths:

### Shrinking

If the requested size is smaller than the current block, the allocator attempts to split the unused portion into a new free block.

### Growing In-Place

If the block immediately following the current allocation is free and provides enough space, the allocator can expand the current block without moving the user's data.

```text
Before:

+-------------+-------------------+
| Allocated A |      Free B       |
+-------------+-------------------+

After realloc:

+-----------------------+---------+
|      Allocated A      |  ...    |
+-----------------------+---------+
```

### Relocation

If the block cannot be expanded in place, a new block is allocated, the existing data is copied, and the old block is released.

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

The allocator obtains virtual memory from the operating system using `mmap` and releases unused pages using `munmap`.

This allows the allocator to manage its own memory regions instead of relying on the standard C allocator.

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

* allocation
* deallocation
* splitting
* coalescing
* `calloc`
* `realloc`
* virtual memory management

### `glHeap.c / glHeap.h`

Implementation of the custom Max Heap used to manage free blocks.

### `test.c`

Tests and examples for exercising the allocator.

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
* Linked structures
* Heaps
* Memory fragmentation
* Block splitting
* Block coalescing
* Data copying with `memcpy`
* Memory initialization with `memset`

## Design Decisions

### Max Heap for Free Blocks

A Max Heap is used to efficiently retrieve a large free block.

This differs from the original course implementation and provided an opportunity to design and integrate a different free-block management structure.

### `mmap` / `munmap`

The allocator uses Linux virtual memory APIs to obtain and release memory regions.

This provides direct interaction with the operating system's virtual memory interface.

## Limitations

This is an educational memory allocator and is **not intended to replace the system allocator**.

Some production-level features are outside the scope of the project, including:

* Thread safety
* Advanced fragmentation mitigation
* Production-grade error handling
* Alignment guarantees for every possible allocation
* Extensive performance optimization
* Full compatibility with the system `malloc` ABI

## Learning Goals

The main goal of the project was to gain practical experience with low-level memory management in C and understand what happens underneath a high-level call such as:

```c
void *ptr = malloc(size);
```

The project explores how an allocator can:

1. Request memory from the operating system.
2. Divide memory into manageable blocks.
3. Track metadata for each block.
4. Find suitable free blocks.
5. Split oversized blocks.
6. Merge adjacent free blocks.
7. Resize allocations.
8. Return unused memory to the operating system.

## Acknowledgements

The initial implementation was developed as part of a memory-management course project.

The project was subsequently extended and modified to experiment with different design choices, including a Max Heap for free-block management, Linux `mmap`/`munmap` based virtual memory management, and a custom `realloc` implementation.
