/* Our node struct holding info on next and prev alloc */
typedef struct alloc {
    size_t size;
    struct alloc *prev;
    struct alloc *next;
} *nodep;

intptr_t align(intptr_t n);

void createnode(nodep prevnode, nodep newnode, nodep nextnode, size_t size);

void *malloc(size_t size);

void *calloc(size_t nmemb, size_t size);

void free(void* ptr);

void *realloc(void *ptr, size_t size);

size_t malloc_usable_size(void *ptr);


