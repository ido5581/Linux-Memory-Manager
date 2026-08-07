#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include "mm.h"

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
        first_vm_page_for_families->vm_page_family[0].struct_size += struct_size;
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
        //vm_page_family_curr->first_page = NULL;
}

void mm_print_registered_page_families(){
    vm_page_family_t *vm_page_family_curr = NULL;
    ITERATE_PAGE_FAMILIES_BEGIN(first_vm_page_for_families, vm_page_family_curr){
        printf("Page Family: %s, Size: %u\n", vm_page_family_curr->struct_name,vm_page_family_curr->struct_size);
    }ITERATE_PAGE_FAMILIES_END(first_vm_page_for_families, vm_page_family_curr);
}

vm_page_family_t* lookup_page_family_by_name(char *struct_name){
    vm_page_family_t *vm_page_family_curr = NULL;
    vm_page_for_families_t* curr_page = first_vm_page_for_families;
    while(curr_page!= NULL){
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


int main(){
    return 0;
}

