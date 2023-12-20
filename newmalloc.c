/* my-malloc.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <pthread.h> //TODO: TEST
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //TODO: TEST

/* Our node struct holding info on next and prev alloc */
typedef struct block {
    size_t size;
    struct block *prev;
    struct block *next;
} block;

/* Alignment function for 16 byte */
intptr_t align(intptr_t n) {
    intptr_t alignment = 16;
    return ((n + alignment - 1) & ~(alignment - 1));
}

/* Information on block & heap */
static block* head = NULL;
static intptr_t _blockSiz = sizeof(block);
static void* startP = NULL;
static void* endP;

/* Function for block creation */
void createBlock(block* prevBlock, block* newBlock, block* nextBlock, size_t _size) {
	newBlock->size = _size;
	newBlock->prev = prevBlock;
	newBlock->next = nextBlock;
	if(prevBlock != NULL) {
		prevBlock->next = newBlock;
	}
	if(nextBlock != NULL) {
		nextBlock->prev = newBlock;
	}
	return;
}

void* splitBlock(block* prevBlock, block* nextBlock, size_t _size) {
	size_t size = align(_size);
	size_t blockSiz = align(_blockSiz);
	intptr_t prevBlockEnd = (intptr_t)prevBlock + blockSiz + align(prevBlock->size);
	intptr_t nextBlockStart = (intptr_t)nextBlock;
	block* newBlock = (block*)prevBlockEnd;
	if(nextBlockStart - prevBlockEnd > size + blockSiz) {
		createBlock(prevBlock, newBlock, nextBlock, _size);
		return newBlock;
	}
	return 0;
}

void* appendBlock(block* prevBlock, size_t _size) {
	size_t blockSiz = align(_blockSiz);
	intptr_t prevBlockEnd = (intptr_t)prevBlock + blockSiz + align(prevBlock->size); 
	block* newBlock = (block*)prevBlockEnd;
	createBlock(prevBlock, newBlock, NULL, _size);
	return newBlock;
}

/* Malloc implementation */
void* malloc(size_t _size) {
	block* newBlock;
	block* curBlock;
	size_t size = align(_size);
	size_t blockSiz = align(_blockSiz);
	size_t membytes = 2 * (size + blockSiz);
	pthread_mutex_lock(&lock); //TODO: TEST
	
	if(startP == NULL) {
		startP = sbrk(0);
	}

	/* Create the head */
	if(head == NULL) {
		if(sbrk(membytes) < (void*)0) {
			pthread_mutex_unlock(&lock); //TODO: TEST
			return NULL;
		}
		endP = sbrk(0);
		head = (block*)align((intptr_t)startP);
		createBlock(NULL, head, NULL, _size);
		pthread_mutex_unlock(&lock); //TODO: TEST
		return (void*)((intptr_t)head + blockSiz);
	}
	
	/* Head already exists -> attempt a split between blocks */
	curBlock = head;
	while(curBlock->next != NULL) {
		if((newBlock = splitBlock(curBlock, curBlock->next, _size)) != (void*)0) {
			pthread_mutex_unlock(&lock); //TODO: TEST
			return (void*)((intptr_t)newBlock + blockSiz);
		}
		curBlock = curBlock->next;
	}
	
	/* Did not manage to split existing block -> see if needs to call sbrk and then can be fit after last block */
	void* lastBlockEnd = (void*)((intptr_t)curBlock + blockSiz + align(curBlock->size));
	if((intptr_t)endP - (intptr_t)lastBlockEnd < blockSiz + size) {
		if(sbrk(membytes) < (void*)0) {
			pthread_mutex_unlock(&lock); //TODO: TEST
			return NULL;
		}
		endP = sbrk(0);
	}
	newBlock = appendBlock(curBlock, _size);
	pthread_mutex_unlock(&lock); //TODO: TEST
	return (void*)((intptr_t)newBlock + blockSiz);
}

/* Free a given pointer */
void free(void* ptr) {
	pthread_mutex_lock(&lock); //TODO: TEST
	size_t blockSiz = align(_blockSiz);
	if(ptr != NULL) {
		block* nodeFree = (block*)((intptr_t)ptr - blockSiz);
		if(nodeFree->next != NULL) {
			nodeFree->next->prev = nodeFree->prev;
		}
		if(nodeFree->prev != NULL) {
			nodeFree->prev->next = nodeFree->next;	
		} else {
			head = nodeFree->next;
		}
	}
	pthread_mutex_unlock(&lock); //TODO: TEST
	return;
}

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

    /* The memory is set to zero */
    memset(newalloc, 0, sizeneeded);
    return newalloc;
}

void *realloc(void *ptr, size_t size) {
	void *newalloc;
	size_t blockSiz = align(_blockSiz);
    block* node = (block*)((intptr_t)(ptr) - blockSiz);
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
        if((intptr_t)node + blockSiz + align(size) > (intptr_t)endP) {
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
    if(((intptr_t)node->next - ((intptr_t)node + blockSiz) >= size)) {
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
	size_t blockSiz = align(_blockSiz);
    block* node = (block*)((intptr_t)(ptr) - align(_blockSiz));
    if(ptr == NULL) {
        return 0;
    }

    /* last node, use the program break(heapend) */
    if(node->next == NULL) {
        return (size_t)((intptr_t)endP - ((intptr_t)node + blockSiz));
    }

    /* in between nodes, use beggining of chunk */
    return (size_t)((intptr_t)node->next - ((intptr_t)node + blockSiz));
}
