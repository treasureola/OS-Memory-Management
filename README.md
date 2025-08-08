# Memory Allocation and Management

Focuses on implementing a power-of-two, constant-time memory allocator!
Supports the general `malloc` and `free` interface, but with a new name (to avoid conflicting with the existing `malloc` and `free`).

## Specification and Interface

Implements:

- `void *p2malloc(int size)`: where the `size` is the requested amount of memory, and the return value is either a pointer to the memory if it was successfully allocated, or `NULL` (`(void *)0`) otherwise.
	You will call `sbrk` if your library needs more memory from the kernel.
	You can find the code for it, and see how it behaves in the `xv6` source.
	Note that it can return `0` if no memory is available.
- `void p2free(void *ptr)`: where `ptr` is a pointer returned from `p2malloc`.

Note that the `p2` prefix stands for "power of two".
When you ask for further memory from `sbrk`, you *must* ask for only a page at a time.
You can assume that `size` will never be greater than `4096 - sizeof(struct p2header)` (see below).

In addition, you *must* implement the following functions that provide some information about what is happening under the hood in your implementation.

- `int p2allocated(void)`: return how much memory is currently allocated.
	This would be the sum of all of the `size` allocations that have not been `free`d (this does not include the memory header, nor the internal fragmentation).
- `int p2totmem(void)`: return the total amount of memory allocated to your library from `sbrk`.


The difference from `p2allocated()` is that  `p2totmem()` returns the total including fragmentation in the system.
This fragmentation includes

1. the memory allocated for the headers,
2. internal fragmentation within each allocation, and
3. external fragmentation due to unused chunks of memory on the freelist.

You can track both of these values with global variables, so you shouldn't need to track each of these forms of fragmentation separately.

## Background

You can see the `xv6` user-level memory allocator (`malloc` and `free`) in `umalloc.c` for an example implementation of memory allocation using a single freelist with some logic for *coalescing* contiguous free holes into a single larger hole. Your implementation only needs coalescing for extra credit.

## Implementation

The header for your memory allocations:

```
struct p2header {
	int size;
};
```

This structure is laid out directly before the actual memory allocations themselves.
So when `p2malloc` returns an address that we'll call `addr`, then the `struct p2header` should be found at address `addr - sizeof(struct p2header)` (assuming `addr` is a `char *`).

**Freelist.**
When memory is freed, it will be added to a freelist corresponding to the power-of-two sized chunk of memory it was initially allocated from.
The freelist structure:

```
struct p2freelist {
	int pow2;
	struct p2freelist *next, *prev;
};
```

When memory is deallocated, you need to track the free chunks of memory, and this list structure does that well.
If you don't think that the `pow2`, nor the `prev` are necessary, then you are on to something.
However, you'll see later that these are needed to support *coalescing* in O(1) time.

**Array of freelists.**
You should maintain a global array of freelists, with one item in the array per power-of-two freelist.
Due to the size of the freelist struct (`sizeof(struct p2freelist) == 12`), you must assume that the smallest freelist tracks allocations of `16` bytes (the next power of two above `12`).
Since you can assume that the user cannot ask for a larger allocation than `4096 - sizeof(struct p2header) = 4092` bytes, that means that your largest freelist tracks memory chunks of size `4096`.
So this means that you'll have an array of size `9` with freelists for `16, 32, 64, 128, 256, 512, 1024, 2048, 4096` sizes.
The `->size` in `p2header` or the `->pow2` in `p2freelist` allow you to figure out which of these freelists the memory or free chunk belongs to.

When an allocation request is made (i.e. `p2malloc(x)`), you want to figure out which freelist to use to serve the request.
You should use the freelist that is for the next highest power of two for `x + sizeof(struct p2header)`.
So a request for `x = 164` bytes should be satisfied from the `256` freelist, `x = 28` from the `32` freelist, and `x = 32` from the `64` freelist.

**`sbrk` and freelist initialization.**
When you don't have any allocations to satisfy a `p2malloc` in a specific freelist, you call `sbrk` to expand the heap.
Once you get the new page from `sbrk`, you should populate the freelist for the size of the current request with the memory.
For example, if you are currently performing an allocation from the size `64` freelist, and need to call `sbrk` to get a page (`4096` bytes), then you should populate your freelist with the portions of that page, thus adding `4096/64 = 64` items to the freelist.
Once this is done, then one of the items from the freelist is returned.

**Tracking allocated and total memory.**
You can use two global variables to track the amount of allocated memory that `sbrk` has given us, and use these to return the corresponding values for `p2allocated` and `p2totmem`, respectively.
