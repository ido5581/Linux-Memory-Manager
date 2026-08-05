#ifndef MM_H
#define MM_H
#include <stdint.h>
#include <stdbool.h>
#define MM_MAX_STRUCT_NAME 32

typedef enum{
    MM_FALSE,
    MM_TRUE,
}vm_bool_t;

typedef struct vm_page_family_{
    char struct_name [MM_MAX_STRUCT_NAME];
    uint32_t struct_size;
} vm_page_family_t;

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
}block_meta_data_t;

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

#define offsetof(struct_name, field_name) ((size_t) &((struct_name *)0)->field_name)

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

void function(block_meta_data_t* first_meta_block){
    int num_of_free_blocks = 0;
    int num_of_allocated_blocks = 0;
    
    block_meta_data_t* free_block =NULL;
    block_meta_data_t* allocated_block = NULL;
    
    for(block_meta_data_t* block = first_meta_block; block!= NULL; block = block->next_block ){
        if(block->is_free == MM_TRUE){
            num_of_free_blocks++;
            if(free_block != NULL || block->block_size > free_block->block_size){
                free_block = block;
            }
        }
        else{// if block is allocated
            num_of_allocated_blocks++;
            if(allocated_block != NULL|| block->block_size > allocated_block->block_size){
                allocated_block = block;
            }
        }
        if(block->next_block != NULL && block->next_block->is_free == MM_TRUE){
            assert(0);
        }       
    } 
    printf("There are %d free blocks \n"
        "and the largest one is: %d bytes\n"
        "its addr: %p", num_of_free_blocks, free_block->block_size,free_block);
    printf("There are %d allocated blocks\n"
        "and the largest one is: %d bytes\n"
        "its addr: %p", num_of_allocated_blocks,allocated_block->block_size,allocated_block);
}
#endif