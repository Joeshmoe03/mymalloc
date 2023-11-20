/* my-malloc.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

/* Linked List Metadata - 12 bytes (doubles are 8, pointers are 8 for 64-bit machines) */
typedef struct {
	
	/* Size of the thing we have allocated. Does not include padding. */
	size_t size;

	/* Pointer to the previous malloc'd thing's node */
	struct nodep *prev;

	/* Pointer to the next malloc'd thing's node */
	struct nodep *next;
} *nodep;

/* When we malloc for the first time, there are no nodes initially and we want to save size of node so we don't call sizeof() excessively */
static nodep head = NULL;
static int nodesiz = sizeof(nodep);

/* Information about our heap size, and future var for tracking end of heap */
static int heapsiz = 2048;
static int EOheap;

/* Function to align input address to the next 16 byte aligned address */
void *aligned (nodep node) { //TODO: FINISH THE REST
	int toAdd = 0;
	if((uintptr_t)node % 16 != 0) {
	toAdd = 16 - (uintptr_t)node % 16;
	return (void*)(nodep)node + toAdd;
}

/* The malloc() function allocates size bytes and returns a pointer
 * to the allocated memory.  The memory is not initialized.  If size
 * is 0, then malloc() returns a unique pointer value that can later
 * be successfully passed to free(). */
void *malloc(size_t size) {
	int heapspace;
	nodep newnode;

	/* We start at our head of the DLL */
	nodep node = head;

	/* If empty, initialize alloc list, 2 NODES, INITIAL and newnode; inside this if, work with INITIAL, outside use newnode */
	if(node == NULL) {

		/* move program break _x_ bytes: Initial memory allocation w/ syscall; sbrk() returns pointer to start of newly allocated memory */
		if ((node = aligned(sbrk(heapsiz))) < 0) {	//TODO: is sbrk aligned?
			return -1;
		}
		node->size = size;
		node->prev = NULL;
		node->next = NULL;
		EOheap = node + heapsiz;
	
		/* Return the aligned address at which alloc'd stuff is */
		return aligned(node + nodesiz);
		}

	/* We already have some thing malloc'd so lets just traverse the DLL until we find next available spot */
	while (node->next != NULL){
		heapspace = node->next - aligned(aligned(node + nodesiz) + (int)node->size); //TODO: ENSURE CALCULATED CORRECTLY
		if (heapspace >= aligned(nodesiz) + aligned((int)size)) {
			newnode = aligned(aligned(node + nodesiz) + (int)node->size);
			newnode->prev = node;
			newnode->next = node->next;
			newnode->size = size;
			node->next = newnode;
			newnode->next->prev = newnode;
			node = newnode;
			return aligned(node + nodesiz);
			//TODO: PUT NODE HERE AND UPDATE NEXT AND PREV
		}
		node = node->next;
	}
	heapspace = EOheap - aligned(aligned(node + nodesiz) + (int)node->size); //TODO: ENSURE CALCULATED CORRECTLY
		
	/* If there is enough space after last node to shove before brk */
	if (heapspace >= (aligned(nodesiz) + aligned((int)size))) {
		newnode = aligned(aligned(node + nodesiz) + (int)node->size);
		newnode->prev = node;
		newnode->next = NULL;
		newnode->size = size;
		node->next = newnode;
		node = newnode;
		return aligned(node + nodesiz);
		//TODO: INSERT NEW NODE IF NO SPACE TO SQUEEZE IN W/ WHILE LOOP
	}
	// increment brk by current sbrksiz + size * 2 ???
	heapsiz = (heapsiz + (int)size) * 2;
	newnode = aligned(sbrk(heapsiz));
	newnode->prev = node;
	newnode->next = NULL;
	node->next = newnode;
	newnode->size = size;
	node = newnode;
	return aligned(node + nodesiz);
	// TODO: GENERAL CASE: call sbrk again to make space. How to handle sbrk failure?
}
	
		
		//Initialize alloc list 
		//(thikning one node at beginning and one node right before program break, both of size 0)

		//node node = head; //right where sbrk returns (beginning of our heap)

			
		// node->isFree = 1;
		// node->size = HEAD - sbrk(0); //start of newly allocated memory - current location of program break
		// node->prev = NULL;
		// node->next = NULL;
		//size? next-prev (only for initial node actually makes sense)

	// ALLOC node = node + structSize + mallocsize (alignment in consideration); //or + 1?


	/* traverse linked list for free chunks of big enough size (alignment considerations) */
//	do {
//
//		//calculation: node->next - node + node->size > size (we wanna malloc)
//		if (node->next - node + node->size > size) { //16 is the fewest we can allocate
//			split(node);
//		}
//
//		
//
//		node->next = node + structSize + pad (32 for both) + mallocsize (alignment in consideration);
//	} while(node->next != NULL);

// 	//break out of this loop if we never have a big enough chunk; call sbrk move program break and ultimately get new free chunk
// 	//update next (same way we alwasy do), size calculation new sbrk call - sbrk(0), isFree

// 	while(//we still can't find big enough free chunk)
// 		//call sbrk() and update stuff

// 	//break out of this loop when we have big enough chunk

// 	//do spliting if too big, otherwise jsut use; then we should be done

// 	if (node->isFree && node->size > size + sizeofStruct) {
// 			split(node);
// 		}

// 	node->next = node + structSize + mallocsize (alignment in consideration);


//	return HEAD;
//}




//     // The free() function frees the memory space pointed to by ptr,
//     // which must have been returned by a previous call to malloc() or
//     // related functions.  Otherwise, or if ptr has already been freed,
//     // undefined behavior occurs.  If ptr is NULL, no operation is
//     // performed.

//void free(void *ptr) {
//
//	/* just update isFree*/
//	
//	/* 4 cases: */
//
//	/* Case 1: NEITHER next/prev is free, just put freed (current) chunk in front */
//
//	/* Case 2: prev is free, combine prev w/ freed (current) doing pointer updating magic dance, then put it in front */
//
//	/* Case 3: next is free, combine next w/ freed (current) doing pointer updating magic dance, then put it in front */
//
//	/* Case 4: next && prev are free, combine BOTH w/ freed (current) doing pointer updating magic dance, then put it in front */
//
//
//	!!!
//
//	//find ptr->next (should get us to next node), updated that next node's prev pointer to whatever ptr->prev is //
//	//find ptr-> prev (should get us to previous node), update that previous node's next pointer to whatever ptr-> nxt is //
//
//}
//
//// void split(ALLOC node) {
//// 	//adjust pointers
//	 
//// }
//
//int main(int argc, char *argv[]) {
//	//printf("work");
//	printf(HEAD);
//	//printf("fake news");
//}
//
//
//
//// //I'm not quite sure what freeing & malloc LOOKS like conceptually
//
//// //sbrk(0) gives us current program break 
//
//// //On success, brk() returns zero.  On error, -1 is returned, and errno is set to ENOMEM.
//
//// // On success, sbrk() returns the previous program break.  (If the
//// //       break was increased, then this value is a pointer to the start of
//// //       the newly allocated memory).  On error, (void *) -1 is returned,
//// //       and errno is set to ENOMEM.
//
//// //malloc should add node
//
//// //free should get rid of data in this node ? (meset?)
//
//// //eventually wanna put stuff back in this area that was freed

int main() {
    int intValue = 42;
    int* intPtr = &intValue;

    // Convert the pointer to an integer using uintptr_t
    //uintptr_t uintptrValue = (uintptr_t)intPtr;

    // Apply modulo operation
    //uintptr_t moduloResult = uintptrValue % 10;  // Replace 10 with your desired divisor

    //printf("Original int value: %d\n", intValue);
    //printf("Pointer value as integer: %lu\n", uintptrValue);
    //printf("Result after applying modulo: %lu\n", moduloResult);

    return 0;
}
