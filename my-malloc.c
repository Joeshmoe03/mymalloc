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
void *malloc(size_t size) {
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
		head = node;

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

/* allocates memory for an array of nmemb elements of size bytes each and returns a pointer to the allocated memory */
void *calloc(size_t nmemb, size_t size) {
	size_t sizeneeded = nmemb * size;

	/* integer overflow case */
	if(__builtin_mul_overflow(nmemb, size, &sizeneeded)) {
		return NULL;
	}
	void *newalloc = malloc(sizeneeded);

	/* if malloc fails, so will calloc */
	if(newalloc == NULL) {
		return NULL;
	}

	nodep node = (nodep)((intptr_t)newalloc - aligned(nodesiz)); 

	/* The memory is set to zero */
	memset((void*)aligned((intptr_t)node + nodesiz), 0, node->size);
	return (void*)aligned((intptr_t)node + nodesiz);
}

void free(void* ptr) {
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

void *realloc(void *ptr, size_t size) {
	void *newalloc;
	nodep node = (nodep)((intptr_t)(ptr) - aligned(nodesiz));
	if(ptr == NULL) {
		return malloc(size);
	}
	if(size == 0) {
		free(ptr);
		return NULL;
	}

	/* last node */
	if(node->next == NULL) {
		/*if our realloc tries to go beyond program break, copy case*/
		if(aligned((intptr_t)node + nodesiz) + size > EOheap) {
			newalloc = malloc(size);
			if(newalloc == NULL) {
				return NULL;
			}
			memcpy(newalloc, ptr, node->size); //size?
			free(ptr);
			return newalloc;
		}

		/* otherwise, extend case */
		node->size = size;
		return ptr;
	}

	/* adjacent memory is free, just "extend" or "shrink" old alloc */
	if(((intptr_t)node->next - aligned((intptr_t)node + nodesiz) >= size)) {
		node->size = size;
		return ptr;
	}

	/* adjacent memory is not free, new malloc of necessary size, copy content from old malloc over to new malloc, free old malloc */
	newalloc = malloc(size);
	/* if malloc fails, so will realloc */
	if(newalloc == NULL) {
		return NULL;
	}
	memcpy(newalloc, ptr, node->size); //size?
	free(ptr);
	return newalloc;
}

size_t malloc_usable_size(void *ptr) {
	nodep node = (nodep)((intptr_t)(ptr) - aligned(nodesiz));
	if(ptr == NULL) {
		return 0;
	}

	/* last node, use the program break(EOheap) */
	if(node->next == NULL) {
		return (size_t)(EOheap - aligned((intptr_t)node + nodesiz));
	}
	
	/* in between nodes, use beggining of chunk */
	return (size_t)((intptr_t)node->next - aligned((intptr_t)node + nodesiz));
}

// int main(int argc, char *argv[]) {
// 	// int* nump = malloc(sizeof(int));
// 	// printf("%p\n", nump);
// 	// *nump = 11023912;
// 	// free(nump);
// 	// char* charp = malloc(10000);
// 	// memset(charp, 1, 10000);
// 	// printf("%p\n", charp);
// 	// char* charp0 = malloc(5000);
// 	// free(charp);
// 	// free(charp0);
	
// 	int* a = (int*)calloc2(4,sizeof(int));
// 	a[0] = 1;
// 	a[1] = 2;
// 	a[2] = 3;
// 	a[3] = 4;
// 	printf("The numbers entered are: ");
//    	for(int i = 0 ; i < 4 ; i++ ) {
//      printf("%d\n", a[i]);
//     }

// 	/* being overwritten weird */
// 	char* letters = malloc(5); 
// 	char* letters2 = malloc(100);
// 	strcpy(letters, "HAHA");
// 	strcpy(letters2, "nooo");
// 	/* same addres for some reason */
// 	printf("%p\n", letters);
// 	printf("%p\n", letters2);
// 	letters = realloc2(letters, 1000); //maybe works, compiler seems to indicate so, but only when using malloc()
// 	strcpy(letters, "HAHAHAHAHA");

	
// 	printf("%s\n", letters);
// 	printf("%s\n", letters2);

// 	return 0;
// }