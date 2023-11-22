/* my-malloc.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Our node struct holding info on next and prev alloc */
typedef struct alloc {
	size_t size;	
	struct alloc *prev;
	struct alloc *next;
} *nodep;

/* Node information for DLL */
static nodep head = NULL;
static intptr_t nodesiz = (intptr_t)sizeof(struct alloc);

/* Heap information */
static intptr_t heapsiz = 2048;
static void* heapstart = NULL;
static void* heapend;

/* See: http://dmitrysoshnikov.com/compilers/writing-a-memory-allocator */
intptr_t align(intptr_t n) {
	intptr_t alignment = 16;
	return ((n + alignment - 1) & ~(alignment - 1));
}

/* Generic function for creating node and updating node metadata */
void createnode(nodep prevnode, nodep newnode, nodep nextnode, size_t size) {
	newnode->size = size;
	newnode->prev = prevnode;
	newnode->next = nextnode;
	if(prevnode != NULL) {
		prevnode->next = newnode;
	}
	if(nextnode != NULL) {
		nextnode->prev = newnode;
	}
	return;
}

/* The malloc() function allocates size bytes and returns a pointer
 * to the allocated memory.  The memory is not initialized.  If size
 * is 0, then malloc() returns a unique pointer value that can later
 * be successfully passed to free(). */
void *malloc(size_t size) {
	intptr_t heapspace;
	intptr_t sbrkval;
	nodep node = head;
	nodep newnode;

	/* If empty, initialize alloc list, 2 NODES, INITIAL and newnode; inside this if, work with INITIAL, outside use newnode */
	if(node == NULL) {
				
		/* If is our first time using malloc */
		heapsiz = (heapsiz < size) ? size * 2 : heapsiz;	
		sbrkval = (intptr_t)sbrk(heapsiz);
		if(sbrkval < 0) {
			return NULL;
		}
		if(heapstart == NULL) {
			heapstart = (void*)align(sbrkval);
		}
		heapend = sbrk(0);
		heapsiz = (intptr_t)heapend - (intptr_t)heapstart;
		
		/* Create the node and set the node metadata */
		newnode = (nodep)align((intptr_t)heapstart);
		createnode(NULL, newnode, NULL, size);
		head = newnode;
		return (void*)(align((intptr_t)newnode + nodesiz));
	}

	/* Can shove before head node? */
	else if((intptr_t)head - (intptr_t)heapstart >= align(align(nodesiz) + size)) {
		newnode = (nodep)heapstart;
		createnode(NULL, newnode, head, size);
		head = newnode;
		return (void*)(align((intptr_t)newnode + nodesiz));
	}

	/* We already have some thing malloc'd so lets just traverse the DLL until we find next available spot */
	while(node->next != NULL) {
		heapspace = (intptr_t)node->next - align(align((intptr_t)node + nodesiz) + node->size);
		if(heapspace >= align(align(nodesiz) + size)) {

			/* Create node and metadata if can fit it in */
			newnode = (nodep)align(align((intptr_t)node + nodesiz) + node->size);
			createnode(node, newnode, node->next, size);
			return (void*)align((intptr_t)newnode + nodesiz);
		}
		node = node->next;
	}
	heapspace = (intptr_t)heapend - align(align((intptr_t)node + nodesiz) + node->size);
		
	/* If there is enough space after last node to shove before brk and fill metadata of node */
	if(heapspace < align(align(nodesiz) + size)) {
		heapsiz = (heapsiz + size) * 2;
		sbrkval = (intptr_t)sbrk(heapsiz);
		if(sbrkval < 0) {
			return NULL;
		}
		heapend = sbrk(0);
		heapsiz = (intptr_t)heapend - (intptr_t)heapstart;
	}
	/* increment brk by current sbrksiz + size * 2, and fill metadata */
	newnode = (nodep)align(align((intptr_t)node + nodesiz) + node->size);
	createnode(node, newnode, NULL, size);
	return (void*)align((intptr_t)newnode + nodesiz);
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

	nodep node = (nodep)((intptr_t)newalloc - align(nodesiz)); 

	/* The memory is set to zero */
	memset(newalloc, 0, sizeneeded);
	return (void*)align((intptr_t)node + nodesiz);
}

void free(void* ptr) {
	if(ptr != NULL) {
		nodep node = (nodep)((intptr_t)(ptr) - align(nodesiz));
		
		/* find node->next (should get us to next node), updated that next node's prev pointer to whatever node->prev is */
		if(node->next != NULL) {
			(node->next)->prev = node->prev; //TODO: THE SOURCE OF ALL MY FRUSTRATIONS!!!!
		}

		/* find node-> prev (should get us to previous node), update that previous node's next pointer to whatever node->nxt is */
		if(node->prev != NULL) {
			(node->prev)->next = node->next;
		} else {
			head = node->next;
		}
		node->next = NULL;
		node->prev = NULL;
	}
	return;
}

void *realloc(void *ptr, size_t size) {
	void *newalloc;
	nodep node = (nodep)((intptr_t)(ptr) - align(nodesiz));
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
		if(align((intptr_t)node + nodesiz) + size > (intptr_t)heapend) {
			newalloc = malloc(size);
			if(newalloc == NULL) {
				return NULL;
			}
			memcpy(newalloc, ptr, size);
			free(ptr);
			return newalloc;
		}

		/* otherwise, extend case */
		node->size = size;
		return ptr;
	}

	/* adjacent memory is free, just "extend" or "shrink" old alloc */
	if(((intptr_t)node->next - align((intptr_t)node + nodesiz) >= size)) {
		node->size = size;
		return ptr;
	}

	/* adjacent memory is not free, new malloc of necessary size, copy content from old malloc over to new malloc, free old malloc */
	newalloc = malloc(size);

	/* if malloc fails, so will realloc */
	if(newalloc == NULL) {
		return NULL;
	}
	memcpy(newalloc, ptr, size);
	free(ptr);
	return newalloc;
}

size_t malloc_usable_size(void *ptr) {
	nodep node = (nodep)((intptr_t)(ptr) - align(nodesiz));
	if(ptr == NULL) {
		return 0;
	}

	/* last node, use the program break(heapend) */
	if(node->next == NULL) {
		return (size_t)((intptr_t)heapend - align((intptr_t)node + nodesiz));
	}
	
	/* in between nodes, use beggining of chunk */
	return (size_t)((intptr_t)node->next - align((intptr_t)node + nodesiz));
}
