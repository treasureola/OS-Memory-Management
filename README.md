# Memory Allocation and Management

This assignment focuses on implementing a power-of-two, constant-time memory allocator!
You'll support the general `malloc` and `free` interface, but with a new name (to avoid conflicting with the existing `malloc` and `free`).

## Specification and Interface

For this assignment, you'll implement:

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

You should not change this structure.
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

You should not change this structure.
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

## Leveling Up!

- **Level 0 (5%).**
	If we write a test file that includes `user.h` (and `types.h` and other normal header includes), that includes an invocation each of the above four functions, your implementation should *compile*.
- **Level 1 (5%).**
	Will successfully be able to repetitively allocate and free memory a single piece of memory with size `12` bytes.
	Essentially works for
	```
	while (1) p2free(p2malloc(12));
	```
	We will use the memory (i.e. `memset` it), so the test case is slightly more complex than above.
	Note that you don't need to have the entire implementation working to satisfy this requirement.
- **Level 2 (40%).**
	Will successfully be able to repetitively allocate and free memory of a single size: `12` bytes, with a maximum of `8` active allocations at any point in time.
	Frees are not in the order of allocation.
	In this setup, after the first allocation, `p2totmem` should always return `4096`.
	Please realize that you don't need to implement all of the above specification for this to be successful, only a single freelist.
	`p2allocated` should return `12 * x` where `x` is the number of active allocations.
	Note that you don't need to implement all freelists to get this level to work.
- **Level 3 (10%).**
	This test makes sure that you're using the proper power-of-two freelist for a sequence of sizes.
	We will do a series of `p2malloc` requests of increasing size from `12` bytes, and increasing by `7` while the value is less than the maximum allowed `size`.
	This will generate a very specific number of `sbrk`s due to allocations for different freelists.
	No `p2free`s will be made.
	We will simply check that the return values of `p2allocated` and `p2totmem` return the correct values.
	This level essentially only tests `p2malloc`.
- **Level 4 (40%).**
	Support all of the specified power of two freelists, and for random allocation and free patterns.
	If we bound the number of allocations in any freelist, we will check that your implementation only allocates actual memory through `sbrk` up to a point, and then never again.
	Should not make unbounded number of `sbrk` requests.
	This level (as opposed to level 3) tests both `p2malloc` *and* `p2free`.

In all of the above, we will overwrite all requested memory to make sure that you are not overlapping your own data-structures with the memory for the allocation.

**Extra Credit.**
Properly support coalescing.
Two free chunks that are contiguous (right next to each other) in the same freelist (say `64` bytes) should be combined into a single free chunk in the `128` byte freelist.
This should only happen when the larger span of free memory would be aligned on its size boundary (i.e. if we are combining into a `128` size free chunk at address `y`, then, `y % 128 == 0`).
This operation is done recursively, thus you may further coalesce `128` byte free chunks into a `256` chunk, etc.

When allocations are made for a given size, and there are no free chunks in the corresponding freelist, you should take an item from the next larger size, and break it up into separate chunks in the current (smaller) freelist.
One of those chunks can be used.
This operation is also done recursively.

- **Level 5 (15%).**
	Properly implement coalescing.
	We will test that you are doing so by ensuring that the heap is not expanded when coalescing should be used.
