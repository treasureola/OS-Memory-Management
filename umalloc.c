#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;
union header {
	struct {
		union header *ptr;
		uint          size;
	} s;
	Align x;
};

struct p2header {
	int size;
};

struct p2freelist { //serves as a node
	int pow2;
	struct p2freelist *next, *prev;
};

int total_allocated = 0;
int total_sbrk = 0;

struct p2freelist *freeArray[9] ; //array of freelist

typedef union header Header;

int called = 0;
static Header  base;
static Header *freep;


void
free(void *ap)
{
	Header *bp, *p;

	bp = (Header *)ap - 1;
	for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
		if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) break;
	if (bp + bp->s.size == p->s.ptr) {
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	} else
		bp->s.ptr = p->s.ptr;
	if (p + p->s.size == bp) {
		p->s.size += bp->s.size;
		p->s.ptr = bp->s.ptr;
	} else
		p->s.ptr = bp;
	freep = p;
}

static Header *
morecore(uint nu)
{
	char *  p;
	Header *hp;

	if (nu < 4096) nu = 4096;
	p = sbrk(nu * sizeof(Header));
	if (p == (char *)-1) return 0;
	hp         = (Header *)p;
	hp->s.size = nu;
	free((void *)(hp + 1));
	return freep;
}

void *
malloc(uint nbytes)
{
	Header *p, *prevp;
	uint    nunits;

	nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
	if ((prevp = freep) == 0) {
		base.s.ptr = freep = prevp = &base;
		base.s.size                = 0;
	}
	for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
		if (p->s.size >= nunits) {
			if (p->s.size == nunits)
				prevp->s.ptr = p->s.ptr;
			else {
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}
			freep = prevp;
			return (void *)(p + 1);
		}
		if (p == freep)
			if ((p = morecore(nunits)) == 0) return 0;
	}
}

void *p2malloc(int size){
	struct p2freelist *node;
	char * addr = "This is ";
	char * extra_mem = "new";
	int which_index = 0;
	int index_size = 16;
	int byte = size + 4; //which index in the free array is it going to be in

	//calculate size of the index it goes to
	while (byte > index_size){ //if the current index size is less than the size passed in + 4
		index_size = index_size * 2; //multiply the size by 2
		which_index++; //increment
	}
	// if (index_size > 4096){
	// 	index_size = 4096;
	// 	which_index = 8;
	// }
	// printf(1, "%s %d\n", "MALLOCED SIZE: ", index_size);
	// printf(1, "%s %d\n", "index SIZE: ", which_index);

	//if no malloc has been called
	if (!freeArray[which_index]){ //if the size of the current index it belongs to is null
		extra_mem = sbrk(4096); // call sbrk
		if (extra_mem == 0){
			return (void *)0;
		}
		total_sbrk = total_sbrk + 4096;
		int mem = 4096/index_size; //divide the 4096 by the size of the index it goes to
		node = (struct p2freelist*)extra_mem; 
		freeArray[which_index] = node;
		node->prev = node->next = 0;
		for (int i=0; i < mem-1; i++){ //assign a node of sizes depending on the index
			node->pow2 = index_size;
			node->next = (struct p2freelist *)((char*)node + index_size);
			node->next->prev = node;
			node = node->next;
		}
	}
	struct p2header *h = (struct p2header *)freeArray[which_index];
	h->size = size;
	addr = (char *)freeArray[which_index] + 4;
	//assign the header by deleting a node
	freeArray[which_index] = freeArray[which_index]->next;
	total_allocated = total_allocated + size;
	return (void *) addr;
}

void p2free(void *ptr){
	struct p2header *h_addr = (struct p2header *)((char*)ptr -4); //get thr header address
	int size = h_addr->size; //the size of the index
	int which_index = 0; //
	int index_size = 16;
	int byte = size + 4; 
	while (byte > index_size){ 
		index_size = index_size * 2;
		which_index++;
	}
	struct p2freelist *node = (struct p2freelist *)h_addr;
	node->pow2 = index_size;
	freeArray[which_index]->prev = node;
	node->next = freeArray[which_index];
	freeArray[which_index] = node;
	total_allocated = total_allocated - size;
}

int p2allocated(void){
	return total_allocated;
}

int p2totmem(void){
	return total_sbrk;
}