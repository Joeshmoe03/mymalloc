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
static size_t nodesiz = sizeof(struct alloc);

/* Information about our heap size, and future var for tracking end of heap */
static intptr_t heapsiz = 2048;
static void* EOheap;

/* SEE: `http://dmitrysoshnikov.com/compilers/writing-a-memory-allocator */
size_t aligned(size_t n) {
	size_t alignment = 16;
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
void *malloc2(size_t size) { //TODO: HANDLE MALLOC(0) + HANDLE IF FIRST MALLOC IS HUGE
	size_t heapspace;
	nodep newnode;

	/* We start at our head of the DLL */
	nodep node = head;

	/* If empty, initialize alloc list, 2 NODES, INITIAL and newnode; inside this if, work with INITIAL, outside use newnode */
	if(node == NULL) {
		
		/* If heapsiz is smaller than what I want to malloc, increase it */
		if((size_t)heapsiz < size) {
			heapsiz = (intptr_t)(aligned(size));
		}

		/* move program break _x_ bytes: Initial memory allocation w/ syscall; sbrk() returns pointer to start of newly allocated memory */
		size_t sbrkresult = (size_t)sbrk(heapsiz);
		
		/* Handle situation of sbrk failure */
		if(sbrkresult < 0) {
			return NULL;
		}
		
		/* Create the node and set the node metadata */
		node = (nodep)aligned(sbrkresult);
		node = createnode(NULL, node, size);

		/* Calculate the end of the brk */
		EOheap = (void*)aligned((size_t)sbrk(0));
	
		/* Return the aligned address at which alloc'd stuff is */
		return (void*)(aligned((size_t)node + nodesiz));
	}

	/* We already have some thing malloc'd so lets just traverse the DLL until we find next available spot */
	while(node->next != NULL) {
		heapspace = (size_t)node->next - aligned(aligned((size_t)node + nodesiz) + node->size);
		if(heapspace >= aligned(nodesiz) + aligned(size)) {

			/* Create node and metadata if can fit it in */
			newnode = (nodep)aligned(aligned((size_t)node + nodesiz) + node->size);
			newnode = createnode(node, newnode, size);
			node = newnode;
			return (void*)aligned((size_t)node + nodesiz);
		}
		node = node->next;
	}
	heapspace = (size_t)EOheap - aligned(aligned((size_t)node + nodesiz) + node->size);
		
	/* If there is enough space after last node to shove before brk and fill metadata of node */
	if(heapspace >= (aligned(nodesiz) + aligned(size))) {
		newnode = (nodep)aligned(aligned((size_t)node + nodesiz) + node->size);
		newnode = createnode(node, newnode, size);
		node = newnode;
		return (void*)aligned((size_t)node + nodesiz);
	}

	/* increment brk by current sbrksiz + size * 2, and fill metadata */
	heapsiz = (heapsiz + (intptr_t)size) * 2;
	newnode = (nodep)aligned((size_t)sbrk(heapsiz));
	newnode = createnode(node, newnode, size);
	node = newnode;
	return (void*)aligned((size_t)node + nodesiz);
}

void free2(void* ptr) {
	/* find ptr-> prev (should get us to previous node), update that previous node's next pointer to whatever ptr-> nxt is */
	nodep node = (nodep)((size_t)(ptr) - aligned(nodesiz));
	if(node->prev != NULL) {
		node->prev->next = node->next;
	}
	
	/* find ptr->next (should get ums to next node), updated that next node's prev pointer to whatever ptr->prev is */
	if(node->next != NULL) {
		node->next->prev = node->prev;
	}
	return;
}

int main(int argc, char *argv[]) {
	int* nump = (int*)malloc2(sizeof(int));
	*nump = 11023912;
	malloc2(100);
	malloc2(1000);
	malloc2(10000);
	free2(nump);
	return 0;
}

