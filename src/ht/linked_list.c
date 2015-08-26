#ifndef LINKED_LIST_C
#define LINKED_LIST_C

#include "linked_list.h"
#include <assert.h>
/**
 * @file linked_list.c
 * 
 * @brief Simple linked list implementation.
 * 
 */

/**
 * Description: Creates a new linked list.
 * 
 * @return A linked list.
 * 
 */
llist_t *create_llist(){
    llist_t *l;
    l = (llist_t *) malloc(sizeof(llist_t));
    assert(l != NULL);

    l->size = 0;
    l->first = NULL;
    l->last = NULL;
    return l;
}


/**
 * Description: Add a pointer to a linked list after a given node.
 * 
 * @param node The node to be put before.
 * @param element The element to be added to the list.
 * 
 * @return The node containing the element.
 */
llist_node_t *add_after_llist_node(llist_t *list,
                                   llist_node_t *node,
                                   void *element){

    if (node == NULL){
        return add_to_llist(list, element);
    }

    assert(node != NULL);
    
    llist_node_t *nnode = (llist_node_t*) malloc(sizeof(llist_node_t));
    nnode->element = element;
    
    nnode->prev  = node;
    nnode->next  = node->next;
    node->next   = nnode;
    
    if (list->last == node){
        list->last = nnode;
    }
    
    list->size++;
    
    return nnode;
}


/**
 * Description: Add a pointer to a linked list before a given node.
 * 
 * @param node The node to be put after.
 * @param element The element to be added to the list.
 * 
 * @return The node containing the element.
 */
llist_node_t *add_before_llist_node(llist_t *list,
                                    llist_node_t *node,
                                    void *element){

    if (node == NULL){
        return add_to_llist(list, element);
    }

    assert(node != NULL);
    
    llist_node_t *nnode = (llist_node_t *) malloc(sizeof(llist_node_t));
    nnode->element = element;
    
    nnode->next  = node;
    nnode->prev  = node->prev;
    node->prev   = nnode;
    
    if (list->first == node){
        list->first = nnode;
    }
    
    list->size++;
    
    return nnode;
}


/**
 * Description: Add a pointer to a linked list.
 * 
 * @param list Linked list.
 * @param element The element to be added to the list.
 * 
 * @return The added node.
 */
llist_node_t *add_to_llist(llist_t *list, void *element){
    llist_node_t *node;
    node = (llist_node_t *) malloc(sizeof(llist_node_t));
    
    assert(node != NULL);
    
    node->element = element;
    node->next = NULL;
    node->prev = NULL;
    
    if ((list->first == NULL) || ((list->last == NULL))){
        list->first = node;
        list->last = node;
    }
    else{
        (list->last)->next = node;
        node->prev = list->last;
        list->last = node;
    }
    
    list->size++;
    
    return node;
}


/**
 * Description: Retrieves an object from the linked list.
 * 
 * @param list Linked list.
 * @param num  The index of the object to be retrieved.
 *
 * @return The pointer introduced in the 'num' position.
 * 
 */
void *llist_element(llist_t *list, int num){
    int i;
    llist_node_t *node;
    
    if (num > list->size){
        node = list->last;

        /* Better search backwards from the end */
        for (i = list->size - 1;(i > num) && (node != NULL);i--){
            node = node->prev;
        }
    }

    else{
        node = list->first;

        /* Better search forwards from the start */
        for (i = 0;(i < num) && (node != NULL);i++){
            node = node->next;
        }
    }
    return node->element;
}


/**
 * Description: Removes an element from the linked list.
 * 
 * @param list The list.
 * @param num The element to be removed.
 * 
 */
int rm_llist_element(llist_t *list, int num){
    int i, e = 0;
    llist_node_t *node;
    node = list->first;

    for (i = 0;(i < num) && (node != NULL);i++){
        node = node->next;
    }

    if (node != NULL){
        if (list->last == node){
            list->last = node->prev;
        }
        
        if (list->first == node){
            list->first = node->next;
        }

        if (node->prev != NULL){
            node->prev->next = node->next;
        }
        
        if (node->next != NULL){
            node->next->prev = node->prev;
        }

        free(node);
        e = 1;
    }
    
    list->size--;
    return e;
}


/**
 * Description: Frees the memory of a linked list.
 * 
 * @param list The list to be freed.
 * @param free_content A pointer to the function to remove
 *                     the contents (or NULL for none).
 * 
 */
void free_llist(llist_t *list, void (*free_content) (void *)){
    if (list == NULL){
        return;
    }
    
    llist_node_t *back, *nod = list->first;
    void *p;

    while (nod != NULL){
        if (free_content != NULL){
            p = nod->element;
            free_content(p);
        }
        back = nod;
        nod = nod->next;
        free(back);
    }

    list->first = NULL;
    list->last  = NULL;
    
    free(list);
}


/**
 * Description: merges two linked lists. Attaches `src` to `dst` and deletes
 * `src` headers.
 * 
 * @param dst Destination list. Its elements will be first.
 * @param src Source list. Its elements will be last, and its headers will be 
 *              deleted.
 * 
 * @return The merged list.
 * 
 */
llist_t *merge_llists(llist_t *dst, llist_t *src){
    // NULL source
    if (src == NULL){
        return dst;
    }
    
    // NULL dest
    if (dst == NULL){
        return src;
    }

    // Link last from dst to first from src
    if (dst->last != NULL){
        dst -> last  -> next = src -> first;
    }
    
    // List first from src to last from dst
    if (src->first != NULL){
        src -> first -> prev = dst -> last;
    }

    // The last one is from src
    if (src->last != NULL){
        dst->last = src->last;
    }
    
    // If dst is empty, link first element to src's one
    if (dst->first == NULL){
        dst->first = src->first;
    }
    
    free(src);

    return dst;
}


#endif
