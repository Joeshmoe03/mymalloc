/* my-malloc.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

// /* Linked List Metadata */


// // typedef struct Header {
// // 	int size; //size (of allocated chunk)
// // } *pHEADER; //pointer to struct node 

// // typedef struct Footer {
// // 	int size; //size of previous chunk (only used if prev is free)
// // } *pFOOTER; //pointer to struct node 

//12 bytes (doubles are 8, pointers are 8 for 64-bit machines)
typedef struct Alloc {
	//int isFree; //whether it's free or not; 1 for free, 0 for NOT free
	double size; //size of empty (free) space
	struct Alloc *next; //pointer to next region
	struct Alloc *prev; //pointer to prev region

} *ALLOC; //pointer to struct Alloc

static ALLOC HEAD = NULL; //need to know where our heap starts but can't use heap (can't use malloc, etc) + every func. needs to know it?


// // The malloc() function allocates size bytes and returns a pointer
// //        to the allocated memory.  The memory is not initialized.  If size
// //        is 0, then malloc() returns a unique pointer value that can later
// //        be successfully passed to free().

void *malloc(size_t size) {

// 	//every time we malloc we make a new node (except for intial: we do 2 nodes?) (current + however much we malloc (however much we need to get to new metadata))
// 	 //should we pad this w/ + 4? (12 + 4 = 16 -> this is where our chunk starts)

// 	//only create a new node  when we split?

	/* If empty, initialize alloc list, 2 NODES, INITIAL and newnode; inside this if, work with INITIAL, outside use newnode */
	if (HEAD == NULL) { //if alloc list is EMPTY
		//move program break _x_ bytes: Initial memory allocation w/ syscall; sbrk() returns pointer to start of newly allocated memory
		HEAD = /*aligned*/sbrk(2048); //first metadata at beggining of heap, make sure that what sbrk returns us is aligned
		
		//Initialize alloc list 
		//(thikning one node at beginning and one node right before program break, both of size 0)

		ALLOC node = HEAD; //right where sbrk returns (beginning of our heap)

		ALLOC last = 2048 - 32;//right before program break (before end of our heap)
	
		// node->isFree = 1;
		// node->size = HEAD - sbrk(0); //start of newly allocated memory - current location of program break
		// node->prev = NULL;
		// node->next = NULL;
		//size? next-prev (only for initial node actually makes sense)
	}

	// ALLOC node = node + structSize + mallocsize (alignment in consideration); //or + 1?


	/* traverse linked list for free chunks of big enough size (alignment considerations) */
	do {

		//calculation: node->next - node + node->size > size (we wanna malloc)
		if (node->next - node + node->size > size) { //16 is the fewest we can allocate
			split(node);
		}

		

		node->next = node + structSize + pad (32 for both) + mallocsize (alignment in consideration);
	} while(node->next != NULL);

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


	return HEAD;
}




//     // The free() function frees the memory space pointed to by ptr,
//     // which must have been returned by a previous call to malloc() or
//     // related functions.  Otherwise, or if ptr has already been freed,
//     // undefined behavior occurs.  If ptr is NULL, no operation is
//     // performed.

void free(void *ptr) {

	/* just update isFree*/
	
	/* 4 cases: */

	/* Case 1: NEITHER next/prev is free, just put freed (current) chunk in front */

	/* Case 2: prev is free, combine prev w/ freed (current) doing pointer updating magic dance, then put it in front */

	/* Case 3: next is free, combine next w/ freed (current) doing pointer updating magic dance, then put it in front */

	/* Case 4: next && prev are free, combine BOTH w/ freed (current) doing pointer updating magic dance, then put it in front */


	!!!

	//find ptr->next (should get us to next node), updated that next node's prev pointer to whatever ptr->prev is //
	//find ptr-> prev (should get us to previous node), update that previous node's next pointer to whatever ptr-> nxt is //

}

// void split(ALLOC node) {
// 	//adjust pointers
	 
// }

void *aligned (ALLOC node) {
	//return aligned address instead of non-aligned address
	//addres mod 16 = remainder
	//return address  + remainder;
	int toAdd = 16 - (int node % 16);


}



int main(int argc, char *argv[]) {
	//printf("work");
	printf(HEAD);
	//printf("fake news");
}



// //I'm not quite sure what freeing & malloc LOOKS like conceptually

// //sbrk(0) gives us current program break 

// //On success, brk() returns zero.  On error, -1 is returned, and errno is set to ENOMEM.

// // On success, sbrk() returns the previous program break.  (If the
// //       break was increased, then this value is a pointer to the start of
// //       the newly allocated memory).  On error, (void *) -1 is returned,
// //       and errno is set to ENOMEM.

// //malloc should add node

// //free should get rid of data in this node ? (meset?)

// //eventually wanna put stuff back in this area that was freed

#include <stdio.h>
#include <stdint.h>

int main() {
    int intValue = 42;
    int* intPtr = &intValue;

    // Convert the pointer to an integer using uintptr_t
    uintptr_t uintptrValue = (uintptr_t)intPtr;

    // Apply modulo operation
    uintptr_t moduloResult = uintptrValue % 10;  // Replace 10 with your desired divisor

    printf("Original int value: %d\n", intValue);
    printf("Pointer value as integer: %lu\n", uintptrValue);
    printf("Result after applying modulo: %lu\n", moduloResult);

    return 0;
}