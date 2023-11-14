/* my-malloc.c */
#include <stdio.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main()
{
  int *i1, *i2;

  printf("sbrk(0) before malloc(4): 0x%x\n", sbrk(0));

  i1 = (int *) malloc(4);
  printf("sbrk(0) after `i1 = (int *) malloc(4)': 0x%x\n", sbrk(0));

  i2 = (int *) malloc(4);
  printf("sbrk(0) after `i2 = (int *) malloc(4)': 0x%x\n", sbrk(0));

  printf("i1 = 0x%x, i2 = 0x%x\n", (unsigned int) i1, (unsigned int) i2);
}

/* Linked List Metadata */

//16 bytes (ints are 4, pointers are 4)
typedef struct node {
    int size; //size of current allocation
    int isfree;
    struct node *next; //pointer to next allocation
    struct node *prev; //pointer to previous allocation

} *NODE; //pointer to struct node 

NODE head = NULL; //global - pointer to the head node //don't wanna use heap memory to intialize this + every function2 needs to know it?


// The malloc() function allocates size bytes and returns a pointer
//        to the allocated memory.  The memory is not initialized.  If size
//        is 0, then malloc() returns a unique pointer value that can later
//        be successfully passed to free().


void *malloc(size_t size) {
    if (head == NULL) {
        //move program break x bytes: Initial memory allocation w/ syscall
        //set HEAD to smth
        int tail = sbrk(135168); //don't need to keep track of end? i feel like we should
        //initialize first node to give to user
        //other node pointing to end
    }
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

