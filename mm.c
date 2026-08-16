#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include "mm.h"

//function declarations
void xfree(void*);
static vm_bool_t mm_split_free_data_block(vm_page_family_t *vm_page_family,
    block_meta_data_t* block_meta_data, uint32_t size);

static vm_page_for_families_t *first_vm_page_for_families = NULL;
static size_t  SYSTEM_PAGE_SIZE = 0;

void mm_init(){
    SYSTEM_PAGE_SIZE = getpagesize();//4KB
}

//Function to request VM memory page
static void* mm_get_new_vm_page_from_kernal(int units){
    char* vm_page = mmap(
        0,
        units*SYSTEM_PAGE_SIZE,
        PROT_READ|PROT_WRITE|PROT_EXEC,
        MAP_ANON|MAP_PRIVATE,
        0,0);
    if(vm_page == MAP_FAILED){
        printf("Error: VM page allocation failed\n");
        return NULL;
    }
    memset(vm_page,0, units*SYSTEM_PAGE_SIZE);
    return (void*)vm_page;
}

//Function to return page to kernal
static void mm_retrun_vm_to_kernal(void* vm_page, int units){
    if(munmap(vm_page, units*SYSTEM_PAGE_SIZE))
        printf("ERROR, couldn't return page to kernal");
}

void mm_instantiate_new_page_family(char* struct_name, uint32_t struct_size){
    vm_page_family_t *vm_page_family_curr = NULL;
    vm_page_for_families_t *new_vm_page_for_famlies = NULL;

    if(struct_size > SYSTEM_PAGE_SIZE){
        printf("Error: structure size exceeds system page size\n");
        return;
    }

    if(!first_vm_page_for_families){//if page is empty
        first_vm_page_for_families = (vm_page_for_families_t*)mm_get_new_vm_page_from_kernal(1);
        first_vm_page_for_families->next = NULL;
        strncpy(first_vm_page_for_families->vm_page_family[0].struct_name,struct_name, MM_MAX_STRUCT_NAME);
        first_vm_page_for_families->vm_page_family[0].struct_size = struct_size;
        first_vm_page_for_families->vm_page_family[0].free_block_heap.size = 0;
        first_vm_page_for_families->vm_page_family[0].free_block_heap.root = NULL;
        first_vm_page_for_families->vm_page_family[0].free_block_heap.offset = offsetof(block_meta_data_t,node);
        return;
    }

    uint32_t count = 0;
    ITERATE_PAGE_FAMILIES_BEGIN(first_vm_page_for_families, vm_page_family_curr){
        if(strncmp(vm_page_family_curr->struct_name,struct_name, MM_MAX_STRUCT_NAME) !=0 ){
            continue;
        }
        assert(0);
    }ITERATE_PAGE_FAMILIES_END(first_vm_page_for_families,vm_page_family_curr);

    if(count == MAX_FAMILIES_PER_VM_PAGE){
        new_vm_page_for_famlies = (vm_page_for_families_t*)mm_get_new_vm_page_from_kernal(1);
        new_vm_page_for_famlies->next = first_vm_page_for_families;
        first_vm_page_for_families = new_vm_page_for_famlies;
        vm_page_family_curr = &first_vm_page_for_families->vm_page_family[0];
    }
    strncpy(vm_page_family_curr->struct_name, struct_name,
        MM_MAX_STRUCT_NAME);
        vm_page_family_curr->struct_size = struct_size;
        vm_page_family_curr->first_page = NULL;
        vm_page_family_curr->free_block_heap.root = NULL;
        vm_page_family_curr->free_block_heap.size = 0;
        vm_page_family_curr->free_block_heap.offset = offsetof(block_meta_data_t,node);
}

void mm_print_registered_page_families(){
    vm_page_family_t *vm_page_family_curr = NULL;
    uint32_t count = 0;
    ITERATE_PAGE_FAMILIES_BEGIN(first_vm_page_for_families, vm_page_family_curr){
        printf("Page Family: %s, Size: %u\n", vm_page_family_curr->struct_name,vm_page_family_curr->struct_size);
    }ITERATE_PAGE_FAMILIES_END(first_vm_page_for_families, vm_page_family_curr);
}

vm_page_family_t* lookup_page_family_by_name(char *struct_name){
    vm_page_family_t *vm_page_family_curr = NULL;
    vm_page_for_families_t* curr_page = first_vm_page_for_families;
    while(curr_page!= NULL){
        uint32_t count = 0;
        ITERATE_PAGE_FAMILIES_BEGIN(curr_page, vm_page_family_curr){
            if(strncmp(vm_page_family_curr->struct_name, struct_name, MM_MAX_STRUCT_NAME) == 0)
            {
                return vm_page_family_curr;
            }
        }ITERATE_PAGE_FAMILIES_END(curr_page, vm_page_family_curr);
        curr_page = curr_page->next;
    }
    return NULL;
}

static void mm_union_free_blocks(block_meta_data_t* first, block_meta_data_t* second){
    assert(first->is_free == MM_TRUE && second->is_free == MM_TRUE);
    first->next_block = second->next_block;
    first->block_size += second->block_size + sizeof(block_meta_data_t);
    if(second->next_block)
        second->next_block->prev_block = first;
}

vm_bool_t mm_is_vm_page_empty(vm_page_t* vm_page){
    if(vm_page->block_meta_data.next_block == NULL &&
    vm_page->block_meta_data.prev_block == NULL &&
    vm_page->block_meta_data.is_free == MM_TRUE){
        return MM_TRUE;
    }
    return MM_FALSE;
}
static inline uint32_t mm_max_page_allocateable_memory(int units){
    return (uint32_t)((SYSTEM_PAGE_SIZE*units)-offsetof(vm_page_t, page_memory));
}

vm_page_t* allocate_vm_page(vm_page_family_t* vm_page_family){
    vm_page_t* vm_page = mm_get_new_vm_page_from_kernal(1);
    MARK_VM_PAGE_EMPTY(vm_page);
    glheap_node_init((&vm_page->block_meta_data.node));
    glnode_insert(&vm_page_family->free_block_heap, &vm_page->block_meta_data.node);
    vm_page->block_meta_data.block_size = mm_max_page_allocateable_memory(1);
    vm_page->block_meta_data.offset = offsetof(vm_page_t, block_meta_data);
    vm_page->next = NULL;
    vm_page->prev = NULL;
    vm_page->page_family = vm_page_family;
    if(vm_page_family->first_page == NULL){
        vm_page_family->first_page = vm_page;
        return vm_page;
    }
    vm_page->next = vm_page_family->first_page;
    vm_page_family->first_page = vm_page;
    return vm_page;
}

void mm_vm_page_delete_and_free(vm_page_t* vm_page){
    vm_page_family_t* vm_page_family = vm_page->page_family;
    if(vm_page_family->first_page == vm_page){
        vm_page_family->first_page = vm_page->next;
        if(vm_page->next != NULL){
            vm_page->next->prev = NULL;
        }
        vm_page->next = NULL;
        vm_page->prev = NULL;
        mm_retrun_vm_to_kernal((void*)vm_page,1);
        return;
    }
    // if the page isnt the first in the family
    vm_page_t* curr_prev = vm_page->prev;
    vm_page_t* curr_next = vm_page->next;
    if(curr_next!= NULL){
        curr_next->prev = curr_prev;
    }
    curr_prev->next = curr_next;

    vm_page->next = NULL;
    vm_page->prev = NULL;
    mm_retrun_vm_to_kernal((void*)vm_page,1);
}

void mm_add_free_block_to_heap(vm_page_family_t *family, block_meta_data_t* free_block){
    assert(free_block->is_free == MM_TRUE);

    glnode_insert(&family->free_block_heap, &free_block->node);
}

void mm_remove_free_block_from_heap(vm_page_family_t *family, block_meta_data_t *free_block) {
    glnode_remove(&family->free_block_heap, &free_block->node);
}

static block_meta_data_t* mm_allocate_free_data_block(vm_page_family_t* vm_page_family, uint32_t req_size){
        block_meta_data_t* biggest_block = mm_get_biggest_block_page_family(vm_page_family);
        if(!biggest_block || biggest_block->block_size < req_size)
            allocate_vm_page(vm_page_family);
        glheap_node_t* biggest_node = gl_heap_extract_max(&vm_page_family->free_block_heap);
        block_meta_data_t* block_to_split = (block_meta_data_t*)((char*)biggest_node - vm_page_family->free_block_heap.offset);
        mm_split_free_data_block(vm_page_family, block_to_split, req_size);

        return block_to_split;
}

void* xcalloc(char* struct_name, int units){
    vm_page_family_t* pg_family = lookup_page_family_by_name(struct_name);
    if(!pg_family){
        printf("page family name not found\n");
        return NULL;
    }

    if(units * pg_family->struct_size > mm_max_page_allocateable_memory(1)){
        printf("size is too big\n");
        return NULL;
    }

    block_meta_data_t* free_block_meta_data = NULL;
    free_block_meta_data = mm_allocate_free_data_block(pg_family, units* pg_family->struct_size);
    if(free_block_meta_data){
        memset((char*)(free_block_meta_data+1),0, free_block_meta_data->block_size);
        return (void*)(free_block_meta_data+1);
    }
    return NULL;
}

void* xrealloc(void* ptr, size_t new_size){
    if(ptr == NULL)
        return NULL;
    
    if(new_size == 0){
        xfree(ptr);
        return NULL;
    }
    block_meta_data_t* my_block = ((block_meta_data_t*)(ptr))-1;
    if(my_block->block_size == new_size){
        printf("there is no need for reallocation. \n");
        return ptr;
    }
    else if(new_size < my_block->block_size){// we're gonna have to split some blocks
        size_t remaining_size = my_block->block_size - new_size; 
        if(remaining_size < sizeof(block_meta_data_t)){
            printf("cannot split\n");
            return ptr;
        }
        else{
            block_meta_data_t* remaining_block = (block_meta_data_t*)((char*)ptr + new_size);
            my_block->block_size = new_size;

            remaining_block->block_size = remaining_size - sizeof(block_meta_data_t);
            remaining_block->is_free = MM_FALSE;
            remaining_block->offset = my_block->offset + new_size + sizeof(block_meta_data_t);
            remaining_block->next_block = my_block->next_block;
            remaining_block->prev_block = my_block;
            my_block->next_block = remaining_block;
            if(remaining_block->next_block){
                remaining_block->next_block->prev_block = remaining_block;
            }
            xfree((void*)(remaining_block+1));
            return ptr;
        }
    }
    else{// new_size > block->size
        block_meta_data_t* next_curr_block =  my_block->next_block;
        if(next_curr_block && next_curr_block->is_free == MM_TRUE && // here we'll combine adjacent blocks
        new_size <= my_block->block_size + sizeof(block_meta_data_t) + next_curr_block->block_size){
            vm_page_t* vm_page = MM_GET_PAGE_FROM_META_BLOCK(next_curr_block);
            mm_remove_free_block_from_heap(vm_page->page_family, next_curr_block);
            my_block->block_size += sizeof(block_meta_data_t) + next_curr_block->block_size;  
            my_block->next_block = next_curr_block->next_block;
            
            if(my_block->next_block) {
                my_block->next_block->prev_block = my_block;
            }
            return ptr;
        }
        else{// here we'll allocate a whole new block
            vm_page_t* vm_page = MM_GET_PAGE_FROM_META_BLOCK(my_block);
            int units = new_size/vm_page->page_family->struct_size;
            void* new_ptr = xcalloc(vm_page->page_family->struct_name, units);
            if(!new_ptr) {
                printf("Relocation failed: Not enough memory.\n");
                return NULL;
            }
            memcpy(new_ptr, ptr, my_block->block_size);
            xfree(ptr);
            return new_ptr;
        }
    }
}

static vm_bool_t mm_split_free_data_block(vm_page_family_t *vm_page_family,
    block_meta_data_t* block_meta_data, uint32_t size){
        uint32_t remaining_size = block_meta_data->block_size - size;
        if(remaining_size < sizeof(block_meta_data_t)){ // Hard internal fragmentation
            block_meta_data->is_free = MM_FALSE;            
        }
        else //if(remaining_size <= sizeof(block_meta_data_t) + vm_page_family->struct_size
            //&& remaining_size >= sizeof(block_meta_data_t)){// Soft internal fragmentation
                {block_meta_data_t* new_meta_block = 
                (block_meta_data_t*)((char*)block_meta_data + sizeof(block_meta_data_t)+size);
                new_meta_block->is_free = MM_TRUE;
                new_meta_block->block_size = (remaining_size - sizeof(block_meta_data_t));
                new_meta_block->offset = block_meta_data->offset+ sizeof(block_meta_data_t) + size;
                glheap_node_init((&new_meta_block->node));
                

                //updating the old block
                
                mm_bind_blocks_for_allocation (block_meta_data, new_meta_block);
                block_meta_data->is_free = MM_FALSE;
                block_meta_data->block_size = size;
                glnode_insert(&vm_page_family->free_block_heap, &new_meta_block->node);
        }
        return MM_TRUE;
    }

void xfree(void* data){
    block_meta_data_t* my_block = ((block_meta_data_t*)(data))-1;
    my_block->is_free = MM_TRUE;
    if(my_block->next_block){
        if(my_block->next_block->is_free == MM_TRUE){
            vm_page_t* my_page_block = (vm_page_t*)MM_GET_PAGE_FROM_META_BLOCK(my_block);
            mm_remove_free_block_from_heap(my_page_block->page_family, my_block->next_block);
            mm_union_free_blocks(my_block, my_block->next_block);
        }
    }
     //vm_page_t* page = (vm_page_t*)((char*)my_block - my_block->offset);
    if(my_block->prev_block){
        if(my_block->prev_block->is_free == MM_TRUE){
            vm_page_t* my_page_block = (vm_page_t*)MM_GET_PAGE_FROM_META_BLOCK(my_block);
            mm_remove_free_block_from_heap(my_page_block->page_family, my_block->prev_block);
            mm_union_free_blocks(my_block->prev_block, my_block);
            my_block = my_block->prev_block;
        }
    }
    vm_page_t* my_page_block = (vm_page_t*)MM_GET_PAGE_FROM_META_BLOCK(my_block);
    vm_bool_t is_page_empty = mm_is_vm_page_empty(my_page_block);
    if(is_page_empty == MM_TRUE){
        mm_vm_page_delete_and_free(my_page_block);
    }
    else{
        mm_add_free_block_to_heap(my_page_block->page_family, my_block);
    }
}
//testing
void mm_print_memory_usage(char* struct_name){
    printf("Page size: %zu\n",SYSTEM_PAGE_SIZE);
    
    vm_page_family_t* vm_page_family_curr = NULL;
    vm_page_for_families_t* curr_page =first_vm_page_for_families;

    while(curr_page!= NULL){
        uint32_t count = 0;
        ITERATE_PAGE_FAMILIES_BEGIN(curr_page, vm_page_family_curr){
            if(struct_name != NULL){
                if(strncmp(vm_page_family_curr->struct_name, struct_name,MM_MAX_STRUCT_NAME) != 0){
                    continue;
                }
            }
            printf("struct name:%s, struct size: %u\n",vm_page_family_curr->struct_name, vm_page_family_curr->struct_size);
            vm_page_t* vm_page =  vm_page_family_curr->first_page;
            ITERATE_VM_PAGE_BEGIN(vm_page_family_curr, vm_page){
                printf("\nprev:%p, next:%p\n",vm_page->prev,  vm_page->next);
                printf("\nfamily name:%s\n",vm_page->page_family->struct_name);
                block_meta_data_t* curr_block;
                ITERATE_VM_PAGE_ALL_BLOCKS_BEGIN(vm_page, curr_block){
                    printf("\n block's address: %p\n", curr_block);
                    if(curr_block->is_free == MM_TRUE){
                        printf("FREE\n");
                    }
                    else{printf("ALLOCATED\n");}
                    printf("block size: %u\n", curr_block->block_size);
                    printf("offset:%u\n",curr_block->offset);
                    printf("prev: %p, next:%p\n", curr_block->prev_block, curr_block->next_block);
                }ITERATE_VM_PAGE_ALL_BLOCKS_END(vm_page, curr_block)
            }ITERATE_VM_PAGE_END(vm_page_family_curr, vm_page)
        }ITERATE_PAGE_FAMILIES_END(curr_page, vm_page_family_curr);
        curr_page = curr_page->next;
    }
}