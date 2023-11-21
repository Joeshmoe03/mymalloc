/* my-malloc.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Linked List Metadata - 12 bytes (doubles are 8, pointers are 8 for 64-bit machines) */
typedef struct alloc {
	
	/* Size of the thing we have allocated. Does not include padding. */
	size_t size;

	/* Pointer to the previous malloc'd thing's node */
	struct alloc *prev;

	/* Pointer to the next malloc'd thing's node */
	struct alloc *next;
} *nodep;

/* When we malloc for the first time, there are no nodes initially and we want to save size of node so we don't call sizeof() excessively */
static nodep head = NULL;
static void* heapstart = NULL;
static intptr_t nodesiz = (intptr_t)sizeof(struct alloc);

/* Information about our heap size, and future var for tracking end of heap */
static intptr_t heapsiz = 2048;
static intptr_t EOheap;

/* SEE: `http://dmitrysoshnikov.com/compilers/writing-a-memory-allocator */
intptr_t aligned(intptr_t n) {
	intptr_t alignment = 16;
	return ((n + alignment - 1) & ~(alignment - 1));
}

/* generic function for creating node and updating node metadata */
nodep createnode(nodep oldnode, nodep newnode, size_t size) {
	newnode->size = size;
	if(oldnode != NULL) {
		newnode->prev = oldnode;
		newnode->next = oldnode->next;
		oldnode->next = newnode;
		if(newnode->next != NULL) {
			newnode->next->prev = newnode;
		}
	} else {
		newnode->prev = NULL;
		newnode->next = NULL;
	}
	return newnode;
}

/* The malloc() function allocates size bytes and returns a pointer
 * to the allocated memory.  The memory is not initialized.  If size
 * is 0, then malloc() returns a unique pointer value that can later
 * be successfully passed to free(). */
void *malloc2(size_t size) {
	intptr_t heapspace;
	nodep newnode;
	intptr_t sbrkval;

	/* We start at our head of the DLL */
	nodep node = head;

	/* If empty, initialize alloc list, 2 NODES, INITIAL and newnode; inside this if, work with INITIAL, outside use newnode */
	if(node == NULL) {
		
		/* If is our first time using malloc */
		if(heapstart == NULL) {
			sbrkval = (intptr_t)sbrk(aligned(heapsiz));
			if(sbrkval < 0) {
				return NULL;
			}
			heapstart = (void*)aligned(sbrkval);
		}

		/* If heapsiz is smaller than what I want to malloc, increase it */
		if(heapsiz < size) {
			sbrkval = (intptr_t)sbrk(aligned(size));
			if(sbrkval < 0) {
				return NULL;
			}
			sbrkval = (intptr_t)sbrk(0);
			heapsiz = (intptr_t)sbrkval;	
		}
		
		/* Create the node and set the node metadata */
		node = (nodep)aligned((intptr_t)heapstart);
		node = createnode(NULL, node, size);

		/* Calculate the end of the brk */
		EOheap = aligned((intptr_t)sbrk(0));
	
		/* Return the aligned address at which alloc'd stuff is */
		return (void*)(aligned((intptr_t)node + nodesiz));
	}

	/* We already have some thing malloc'd so lets just traverse the DLL until we find next available spot */
	while(node->next != NULL) {
		heapspace = (intptr_t)node->next - aligned(aligned((intptr_t)node + nodesiz) + node->size);
		if(heapspace >= aligned(nodesiz) + aligned((intptr_t)size)) {

			/* Create node and metadata if can fit it in */
			newnode = (nodep)aligned(aligned((intptr_t)node + nodesiz) + node->size);
			newnode = createnode(node, newnode, size);
			node = newnode;
			return (void*)aligned((intptr_t)node + nodesiz);
		}
		node = node->next;
	}
	heapspace = EOheap - aligned(aligned((intptr_t)node + nodesiz) + node->size);
		
	/* If there is enough space after last node to shove before brk and fill metadata of node */
	if(heapspace >= (aligned(nodesiz) + aligned(size))) {
		newnode = (nodep)aligned(aligned((intptr_t)node + nodesiz) + node->size);
		newnode = createnode(node, newnode, size);
		node = newnode;
		return (void*)aligned((intptr_t)node + nodesiz);
	}

	/* increment brk by current sbrksiz + size * 2, and fill metadata */
	heapsiz = (heapsiz + size) * 2;
	sbrkval = (intptr_t)sbrk(aligned(heapsiz));
	if(sbrkval < 0) {
		return NULL;
	}
	newnode = (nodep)aligned(sbrkval);
	newnode = createnode(node, newnode, size);
	node = newnode;
	return (void*)aligned((intptr_t)node + nodesiz);
}

void *calloc2(size_t nmemb, size_t size) {
	/* integer overflow case */

	/* allocates memory for an array of nmemb elements of size bytes each and returns a pointer to the allocated memory */
	size_t sizeneeded = nmemb * size;
	nodep node = (nodep)((size_t)malloc2(sizeneeded) - aligned(nodesiz)); 

	/* The memory is set to zero */
	memset((void*)aligned((intptr_t)node + nodesiz), 0, node->size);

	/* if malloc fails, so will calloc */
	if(malloc2 == NULL) {
		return NULL;
	}

	return (void*)aligned((intptr_t)node + nodesiz);
}

void *realloc2(void *ptr, size_t size) {
	if(ptr == NULL) {
		malloc2(size);
	}
	if(size == 0) { //implied: && ptr != NULL
		free2(ptr);
	}
	nodep node = (nodep)((size_t)(ptr) - aligned(nodesiz));
	
	/* Case 1: new size < old size */
	if(size < node->size) {
		node->size = size;
		return ptr;
	}

	/* Case 2: new size > old size */

	/* Case 2a: adjacent memory is free, just "extend" old alloc */
	if(((intptr_t)node->next - aligned(aligned((intptr_t)node + nodesiz) + node->size) > aligned(size - node->size))) { //CHECK CALC.
		node->size = size;
		return ptr;
	}

	/* Case 2b: adjacent memory is not free, new malloc of necessary size, copy content from old malloc over to new malloc, free old malloc */
	void* newalloc = malloc2(size);
	memcpy(newalloc, ptr, node->size);
	free2(ptr);
	return newalloc;
}

void free2(void* ptr) {
	nodep node = (nodep)((intptr_t)(ptr) - aligned(nodesiz));
	if(ptr != NULL){
		/* find node-> prev (should get us to previous node), update that previous node's next pointer to whatever node->nxt is */
		if(node->prev != NULL) {
			node->prev->next = node->next;
		} else {
			head = NULL;
		}
		
		/* find node->next (should get us to next node), updated that next node's prev pointer to whatever node->prev is */
		if(node->next != NULL) {
			node->next->prev = node->prev;
		}
	}
	return;
}

int main(int argc, char *argv[]) {
	// int* nump = malloc2(sizeof(int));
	// printf("%p\n", nump);
	// *nump = 11023912;
	// free2(nump);
	// char* charp = malloc2(10000);
	// memset(charp, 1, 10000);
	// printf("%p\n", charp);
	// char* charp0 = malloc2(5000);
	// free2(charp);
	// free2(charp0);
	int* a = (int*)calloc2(4,sizeof(int));
	a[0] = 1;
	a[1] = 2;
	a[2] = 3;
	a[3] = 4;
	printf("The numbers entered are: ");
   	for(int i = 0 ; i < 4 ; i++ ) {
      printf("%d", a[i]);
    }

	char* letters = malloc2(5);
	letters = realloc2(letters, 10);

	return 0;
}

