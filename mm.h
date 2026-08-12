#ifndef __MM_H
#define __MM_H
#include "glHeap.h"
#include <stdint.h>
#include <stdbool.h>
#define MM_MAX_STRUCT_NAME 32

typedef enum{
    MM_FALSE,
    MM_TRUE,
}vm_bool_t;


typedef struct vm_page_for_families_{
    struct vm_page_for_families_ *next;
    vm_page_family_t vm_page_family[0];
}vm_page_for_families_t;


typedef struct block_meta_data_{
    vm_bool_t is_free;
    uint32_t block_size;
    uint32_t offset;
    struct block_meta_data_*prev_block;
    struct block_meta_data_*next_block;
    glheap_node_t node;
}block_meta_data_t;

typedef struct vm_page_family_{
    char struct_name [MM_MAX_STRUCT_NAME];
    uint32_t struct_size;
    struct vm_page_t* first_page;
    glheap_t free_block_heap;
} vm_page_family_t;

typedef struct vm_page_{
    struct vm_page_* next;
    struct vm_page_* prev;
    vm_page_family_t* page_family;
    block_meta_data_t block_meta_data;
    char page_memory[0];
}vm_page_t;

vm_page_family_t* lookup_page_family_by_name(char *struct_name);

#define MAX_FAMILIES_PER_VM_PAGE    \
    ((SYSTEM_PAGE_SIZE - sizeof(vm_page_for_families_t*))/\
    sizeof(vm_page_family_t))

#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_for_families_ptr, curr)\
{\
    uint32_t count = 0;\
    for(curr = (vm_page_family_t*)&vm_page_for_families_ptr->vm_page_family[0];\
    curr->struct_size && count < MAX_FAMILIES_PER_VM_PAGE;\
    curr++, count++){

#define ITERATE_PAGE_FAMILIES_END(vm_page_families_ptr, curr)}}
#ifndef offsetof
#define offsetof(struct_name, field_name) ((size_t) &((struct_name *)0)->field_name)
#endif

#define MM_GET_PAGE_FROM_META_BLOCK(block_meta_data_ptr)\
(void*)((unsigned long)(block_meta_data_ptr)&~(SYSTEM_PAGE_SIZE-1))

#define NEXT_META_BLOCK_BY_SIZE(block_meta_data_ptr)\
(block_meta_data_t*)(((char*)(block_meta_data_ptr+1))+block_meta_data_ptr->block_size)

#define NEXT_META_BLOCK(block_meta_data_ptr)\
(block_meta_data_ptr->next_block)

#define PREV_META_BLOCK(block_meta_data_ptr)\
((block_meta_data_ptr->prev_block))


#define mm_bind_blocks_for_allocation(allocated_meta_block, free_meta_block) \
{\
    free_meta_block->next_block = allocated_meta_block->next_block;\
    if(free_meta_block->next_block != NULL){\
        free_meta_block->next_block->prev_block =free_meta_block;\
    }\
    free_meta_block->prev_block = allocated_meta_block;\
    allocated_meta_block->next_block = free_meta_block;\
}

vm_bool_t mm_is_vm_page_empty(vm_page_t* vm_page);

#define MARK_VM_PAGE_EMPTY(vm_page_t_ptr)\
vm_page_t_ptr->next = NULL;\
vm_page_t_ptr->prev = NULL;\
vm_page_t_ptr->block_meta_data.is_free = MM_TRUE

//vm_page_t* curr = NULL;
#define ITERATE_VM_PAGE_BEGIN(vm_page_family_ptr, curr)\
{\
    for(curr = (vm_page_family_ptr)->first_page; curr != NULL; curr = curr->next){

#define ITERATE_VM_PAGE_END(vm_page_family_ptr, curr)\
    }\
}

#define ITERATE_VM_PAGE_ALL_BLOCKS_BEGIN(vm_page_ptr, curr)\
{\
    for(curr =(vm_page_ptr)->block_meta_data; curr != NULL; curr = curr->next_block){        
        
#define ITERATE_VM_PAGE_ALL_BLOCKS_END(vm_page_ptr, curr)\
    }\
}

vm_page_t* allocate_vm_page(vm_page_family_t* vm_page_family);
    
void mm_vm_page_delete_and_free(vm_page_t* vm_page);

void mm_add_free_block_to_heap(vm_page_family_t *family, block_meta_data_t *free_block);


//PEEK
static inline block_meta_data_t* mm_get_biggest_block_page_family(vm_page_family_t *vm_page_family){
    if(!vm_page_family || vm_page_family->free_block_heap.size == 0 || !vm_page_family->free_block_heap.root)
        return NULL;
    glheap_node_t* root = vm_page_family->free_block_heap.root;
    return (block_meta_data_t*)((char*)root- vm_page_family->free_block_heap.offset);
}

#define IS_FREE_BLOCK_HEAP_EMPTY(family)(family->free_block_heap.size==0)

#endif