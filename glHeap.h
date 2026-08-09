#ifndef __GLHEAP__
#define __GLHEAP__
#include <stddef.h>

typedef struct glheap_node_{
    struct glHeap_node_ *parent; 
    struct glHeap_node_ *left;
    struct glHeap_node_ *right;
}glheap_node_t;

typedef struct glheap_{// the "manager of the heap"
    glheap_node_t* head;
    unsigned int offset;
    unsigned int size; // current size of the heap
}glheap_t;

void glheap_insert(glheap_t* head, glheap_node_t* glnode);
glheap_node_t* gl_heap_extract_max(glheap_t* glheap);
void glheap_remove(glheap_t* glheap);

#define glheap_node_init(glnode) \
{                                \
    glnode->parent = NULL;       \
    glnode->left = NULL;         \
    glnode->right = NULL;        \
}

void glheap_init(glheap_node_t* glheap, unsigned int offset);

#endif