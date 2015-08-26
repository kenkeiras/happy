#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
/**
 * @file linked_list.h
 * 
 * @brief Double linked list
 * 
 */

/**
 * Linked list node
 *
 */
typedef struct llist_node{
    void *element;

    struct llist_node *next;
    struct llist_node *prev;
}llist_node_t;


/**
 * Linked list type
 * 
 */
typedef struct{
    int size;
    llist_node_t *first;
    llist_node_t *last;
}llist_t;


/**
 * Description: Creates a new linked list
 * 
 * @return A linked list
 */
llist_t *create_llist();


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
                                   void *element);


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
                                    void *element);


/**
 * Description: Add a pointer to a linked list.
 * 
 * @param list Linked list.
 * @param element The element to be added to the list.
 * 
 * @return The added node.
 */
llist_node_t *add_to_llist(llist_t *list, void *element);


/**
 * Description: Retrieves an object from the linked list.
 * 
 * @param list Linked list.
 * @param num  The index of the object to be retrieved.
 *
 * @return The pointer introduced in the 'num' position.
 * 
 */
void *llist_element(llist_t *list, int num);


/**
 * Description: Frees the memory of a linked list.
 * 
 * @param list The list to be freed.
 * @param free_content A pointer to the function to remove
 *                     the contents (or NULL for none).
 */
void free_llist(llist_t *list, void (*free_content) (void *));


/**
 * Description: merges two linked lists. Attaches `src` to `dst` and may delete
 * `src` headers.
 * 
 * @param dst Destination list. Its elements will be first.
 * @param src Source list. Its elements will be last, and its headers may be 
 *              deleted.
 * 
 * @return The merged list.
 * 
 */
llist_t *merge_llists(llist_t *dst, llist_t *src);

#endif
