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
static void swap_nodes(glheap_t* heap, glheap_node_t* child, glheap_node_t* parent){// works only for father and child
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

static void shift_up(glheap_t* heap, glheap_node_t* glnode){
    while(glnode->parent != NULL && compare_nodes(heap, glnode, glnode->parent)>0){
        swap_nodes(heap, glnode, glnode->parent);    
    }
    return;
}

static glheap_node_t* get_insertion_parent(glheap_t* heap){
    unsigned int target = heap->size+1;
    int MSB =-1;
    for(int i = 31; i >=0; i--){
        if((target >> i)&1){
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

static glheap_node_t* get_last_node(glheap_t* heap){
    unsigned int target = heap->size;
    int MSB =-1;
    for(int i = 31; i >=0; i--){
        if((target >> i)&1){
            MSB = i;
            break;
        }
    }
    glheap_node_t* curr = heap->root;
    for(int i = MSB-1; i >= 0; i--){
         if((target >> i)&1){
            curr = curr->right;
         }
          else{
            curr = curr->left;
          }
    }
    return curr;
}

static void shift_down(glheap_t* glheap, glheap_node_t* glnode){
    if(glnode == NULL) return;
    while(1){
        glheap_node_t* biggest_child = NULL;

        if(glnode->left != NULL && glnode->right != NULL){
            if(compare_nodes(glheap, glnode->left, glnode->right)>0)
                biggest_child = glnode->left;
            else
                biggest_child = glnode->right;
        }
        else if(glnode->left != NULL){
            biggest_child = glnode->left;
        }
        else if(glnode->right != NULL)
            biggest_child = glnode->right;
        if(!biggest_child)
            break;
        
        if(compare_nodes(glheap, biggest_child, glnode) > 0)
            swap_nodes(glheap, biggest_child, glnode);
        else
            break;
    }
}

glheap_node_t* gl_heap_extract_max(glheap_t* glheap){
    if(glheap->size == 0)
        return NULL;
        
    glheap_node_t* root = glheap->root;
    if(glheap->size == 1){
        glheap->root = NULL;
        glheap->size = 0;
        root->left = NULL;
        root->right = NULL;
        root->parent = NULL;
        return root;
    }
    glheap_node_t* bottom = get_last_node(glheap);
    glheap_node_t* root_left = glheap->root->left;
    glheap_node_t* root_right = glheap->root->right;
    if(root_left == bottom) root_left =NULL;
    if(root_right == bottom) root_right = NULL;
    
    if(bottom == bottom->parent->right){
        bottom->parent->right = NULL;
    }
    else{
        bottom->parent->left = NULL;
    }
    bottom->left = root_left;
    if(root_left)
        root_left->parent = bottom;

    bottom->right = root_right;
    if(root_right)
        root_right->parent = bottom;

    bottom->parent = NULL;
    
    glheap->root = bottom;

    glheap->size--;
    shift_down(glheap, bottom);
    root->left = NULL;
    root->right = NULL;
    root->parent = NULL;
    return root;
}

void glnode_remove(glheap_t* glheap, glheap_node_t* glnode){
    if(!glheap || !glnode || glheap->size == 0) return;
    glheap_node_t* bottom = get_last_node(glheap);
    if(bottom == glnode){
        if(bottom->parent){
            if(bottom->parent->left == bottom)
                bottom->parent->left = NULL;
            else
                bottom->parent->right = NULL;
        }
        else{
            glheap->root = NULL;
        }
        glheap->size--;
        return;
    }
    if (bottom->parent->left == bottom) {
        bottom->parent->left = NULL;
    } else {
        bottom->parent->right = NULL;
    }
    glheap->size--;
    bottom->parent = glnode->parent;
    bottom->left = glnode->left;
    bottom->right = glnode->right;
    if (bottom->left) {
        bottom->left->parent = bottom;
    }
    if (bottom->right) {
        bottom->right->parent = bottom;
    }
    if (bottom->parent) {
        if (bottom->parent->left == glnode) {
            bottom->parent->left = bottom;
        } else {
            bottom->parent->right = bottom;
        }
    } else {glheap->root = bottom;}
    if (bottom->parent != NULL && compare_nodes(glheap, bottom, bottom->parent) > 0) {
        shift_up(glheap, bottom);
    } else {
        shift_down(glheap, bottom);
    } 
}