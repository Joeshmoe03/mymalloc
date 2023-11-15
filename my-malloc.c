/* my-malloc.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

/* Linked List Metadata */


typedef struct Header {
	int size; //size (of allocated chunk)
} *pHEADER; //pointer to struct node 

typedef struct Footer {
	int size; //size of previous chunk (only used if prev is free)
} *pFOOTER; //pointer to struct node 

//12 bytes (ints are 4, pointers are 4)
typedef struct Flist {
int size; //size of empty (free) space
	struct flist *next; //pointer to next free region
	struct flist *prev; //pointer to prev free region
} *pFLIST; //pointer to struct flist

static HEAD = NULL; //global //don't wanna use heap memory to intialize this + every function2 needs to know it?


// The malloc() function allocates size bytes and returns a pointer
//        to the allocated memory.  The memory is not initialized.  If size
//        is 0, then malloc() returns a unique pointer value that can later
//        be successfully passed to free().


void *malloc(size_t size) {
	if (HEAD == NULL) { //if free list is EMPTY
		//move program break _x_ bytes: Initial memory allocation w/ syscall
		sbrk(250);
	}

	/* traverse free list for big enough chunk */

	/* if we find big enough chunk, use it */

		/* if it's too big (measure for this if size * 2 < free space, use half for malloc and stash other half?), split it, then use it */ -> func for this? 
	
	/* if we don't, call sbrk or brk to move program break and ultimately get new free chunk, use it */

		/* if it's too big, split it, then use it */ -> func for this?


	//int tail = sbrk(135168); //don't need to keep track of end? i feel like we should
	//initialize first node to give to user
	//other node pointing to end
}




    // The free() function frees the memory space pointed to by ptr,
    // which must have been returned by a previous call to malloc() or
    // related functions.  Otherwise, or if ptr has already been freed,
    // undefined behavior occurs.  If ptr is NULL, no operation is
    // performed.

void free(void *ptr) {
	/* Insert at front w/ LIFO */

	/* 4 cases: */

	/* Case 1: NEITHER next/prev is free, just put freed (current) chunk in front */

	/* Case 2: prev is free, combine prev w/ freed (current) doing pointer updating magic dance, then put it in front */

	/* Case 3: next is free, combine next w/ freed (current) doing pointer updating magic dance, then put it in front */

	/* Case 4: next && prev are free, combine BOTH w/ freed (current) doing pointer updating magic dance, then put it in front */


}




int main(int argc, char *argv[]) {
	printf("work");
}



//I'm not quite sure what freeing & malloc LOOKS like conceptually

//sbrk(0) gives us current program break 

//On success, brk() returns zero.  On error, -1 is returned, and errno is set to ENOMEM.

// On success, sbrk() returns the previous program break.  (If the
//       break was increased, then this value is a pointer to the start of
//       the newly allocated memory).  On error, (void *) -1 is returned,
//       and errno is set to ENOMEM.

//malloc should add node

//free should get rid of data in this node ? (meset?)

//eventually wanna put stuff back in this area that was freed

