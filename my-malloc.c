/* my-malloc.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

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
static size_t nodesiz = sizeof(nodep);

/* Information about our heap size, and future var for tracking end of heap */
static intptr_t heapsiz = 2048;
static void* EOheap;

/* SEE: `http://dmitrysoshnikov.com/compilers/writing-a-memory-allocator */
size_t aligned(size_t n) {
	size_t alignment = 16;
	return ((n + alignment - 1) & ~(alignment - 1));
}

/* The malloc() function allocates size bytes and returns a pointer
 * to the allocated memory.  The memory is not initialized.  If size
 * is 0, then malloc() returns a unique pointer value that can later
 * be successfully passed to free(). */
void *malloc2(size_t size) {
	size_t heapspace;
	nodep newnode;

	/* We start at our head of the DLL */
	nodep node = head;

	/* If empty, initialize alloc list, 2 NODES, INITIAL and newnode; inside this if, work with INITIAL, outside use newnode */
	if(node == NULL) {

		/* move program break _x_ bytes: Initial memory allocation w/ syscall; sbrk() returns pointer to start of newly allocated memory */
		
		node = (nodep)aligned((size_t)sbrk(heapsiz));
		//if (node = aligned(sbrk(heapsiz))) {	//TODO: is sbrk aligned?
		//	return NULL;
		//}
		node->size = size;
		node->prev = NULL;
		node->next = NULL;
		EOheap = (void*)aligned((size_t)sbrk(0)); //node + heapsiz;
	
		/* Return the aligned address at which alloc'd stuff is */
		return (void*)aligned((size_t)node + nodesiz);
		}

	/* We already have some thing malloc'd so lets just traverse the DLL until we find next available spot */
	while (node->next != NULL){
		heapspace = (size_t)node->next - aligned(aligned((size_t)node + nodesiz) + node->size); //TODO: ENSURE CALCULATED CORRECTLY
		if (heapspace >= aligned(nodesiz) + aligned(size)) {
			newnode = (nodep)aligned(aligned((size_t)node + nodesiz) + node->size);
			newnode->prev = node;
			newnode->next = node->next;
			newnode->size = size;
			node->next = newnode;
			newnode->next->prev = newnode;
			node = newnode;
			return (void*)aligned((size_t)node + nodesiz);
			//TODO: PUT NODE HERE AND UPDATE NEXT AND PREV
		}
		node = node->next;
	}
	heapspace = (size_t)EOheap - aligned(aligned((size_t)node + nodesiz) + node->size); //TODO: ENSURE CALCULATED CORRECTLY
		
	/* If there is enough space after last node to shove before brk */
	if (heapspace >= (aligned(nodesiz) + aligned(size))) {
		newnode = (nodep)aligned(aligned((size_t)node + nodesiz) + node->size);
		newnode->prev = node;
		newnode->next = NULL;
		newnode->size = size;
		node->next = newnode;
		node = newnode;
		return (void*)aligned((size_t)node + nodesiz);
		//TODO: INSERT NEW NODE IF NO SPACE TO SQUEEZE IN W/ WHILE LOOP
	}
	// increment brk by current sbrksiz + size * 2 ???
	heapsiz = (heapsiz + (intptr_t)size) * 2;
	newnode = (nodep)aligned((size_t)sbrk(heapsiz));
	newnode->prev = node;
	newnode->next = NULL;
	node->next = newnode;
	newnode->size = size;
	node = newnode;
	return (void*)aligned((size_t)node + nodesiz);
	// TODO: GENERAL CASE: call sbrk again to make space. How to handle sbrk failure?
}

int main(int argc, char *argv[]) {
	int* nump = (int*)malloc2(sizeof(int));
	*nump = 11023912;
	printf("%d\n", *nump);
	malloc2(100);
	malloc2(1000);
	malloc2(10000);
	return 0;
}

