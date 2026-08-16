# Custom Memory Manager (Linux OS Simulation)

## Overview
A custom memory allocation library written in C that simulates core Linux OS memory management mechanisms. This project replaces standard dynamic memory functions with a robust system that handles page allocation, block splitting, and memory fragmentation prevention. It provides deep visibility into how memory is structured and managed under the hood.

## Core Features
*   **Custom Allocators:** Full implementation of `xcalloc`, `xfree`, and a highly optimized `xrealloc`.
*   **Page Families:** Organizes memory allocations by data structures, mapping specific sizes to dedicated virtual memory pages.
*   **Intrusive Doubly Linked Lists:** Tracks free and allocated blocks directly within memory headers, eliminating the need for external tracking structures.
*   **Dynamic Splitting & Coalescing:** Automatically splits large memory blocks to accommodate smaller requests, and merges (coalesces) adjacent free blocks upon deletion to prevent fragmentation.
*   **In-Place Expansion:** `xrealloc` is optimized to expand blocks in-place by consuming adjacent free space, falling back to memory relocation (`memcpy`) only when necessary.

## Architecture

The memory manager is built on a custom metadata structure (`block_meta_data_t`) prefixed to every data block. It tracks:
* Block size and starting offset from the page base.
* Allocation status (`is_free` flag).
* Pointers to adjacent blocks (`next_block`, `prev_block`) for O(1) coalescence.

### Memory Layout
```text
[ Page Header (vm_page_t) ] [ Meta Data | User Data ] [ Meta Data | User Data ] ...

##Getting Started
