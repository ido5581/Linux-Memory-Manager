#include "glHeap.h"
#include "mm.h"

static int compare_nodes(glheap_t* heap, glheap_node_t* n1, glheap_node_t* n2){
    block_meta_data_t* block1 = (block_meta_data_t*)((char*)n1-heap->offset);
    block_meta_data_t* block2 = (block_meta_data_t*)((char*)n2-heap->offset);
    uint32_t n1_size =block1->block_size;
    uint32_t n2_size = block2->block_size;
    if(n1_size > n2_size) return 1; // means our node(n1) is bigger than the node we compared to (n2)

    else if (n1_size < n2_size)
        return -1;

    else return 0;
}

static void shift_up(glheap_t* heap, glheap_node_t* glnode){
    while(glnode->parent != NULL && compare_nodes(heap, glnode, glnode->parent)){
        swap_node(heap, glnode, glnode->parent);    
    }
    return;
}

static void swap_nodes(glheap_t* heap, glheap_node_t* child, glheap_node_t* parent){
    glheap_node_t* child_left = child->left;
    glheap_node_t* child_right = child->right;
    glheap_node_t* grandparent = parent->parent;
    glheap_node_t* brother = NULL;
    if(child == parent->left){
        brother = parent->right;
        child->right = brother;
        child->left = parent;
    }
    else{//child = parent->right
        brother = parent->left;
        child->right = parent;
        child->left = brother;
    }
    if(grandparent){
        if(parent == grandparent->left){
            grandparent->left = child;
        }
        else{
            grandparent->right = child;
        }
    }
    else{
        heap->root = child;
    }
    child->parent = grandparent;
    if(brother)
        brother->parent = child;
    
    parent->parent = child;
    parent->left = child_left;
    if(child_left)
        child_left->parent = parent;
    parent->right = child_right;
    if(child_right)
        child_right->parent = parent;
}

static glheap_node_t* get_insertion_parent(glheap_t* heap){
    unsigned int target = heap->size+1;
    uint32_t MSB =-1;
    for(int i = 31; i >=0; i++){
        if((target >> i)&1 == 1){
            MSB = i;
            break;
        }
    }
    glheap_node_t* curr = heap->root;
    for(int i = MSB-1; i > 0; i--){
         if((target >> i)&1){
            curr = curr->right;
         }
          else{
            curr = curr->left;
          }
    }
    return curr;
}

void glnode_insert(glheap_t* heap, glheap_node_t* glnode){
    glnode->left = NULL;
    glnode->right = NULL;
    glnode->parent = NULL;

    if(heap->size == 0){ 
        heap->root = glnode;
        heap->size++;
        return;
    }
    glheap_node_t* father = get_insertion_parent(heap);
    if(father->left == NULL){
        father->left = glnode;
    }
    else{
        father->right = glnode;
    }
    glnode->parent = father;
    heap->size++;
    shift_up(heap,glnode);
}




