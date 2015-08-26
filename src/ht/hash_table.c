#ifndef HASH_TABLE_C
#define HASH_TABLE_C

#include "hash_table.h"

/**
 * @file hash_table.c
 *
 * @brief Almost Ansi C hash table implementation.
 *
 * An ANSI C implementation of a hash table  allowing to use integers and
 * '\0' ended strings as index.
 *
 * @note Uses One-at-a-Time hash algorithm for strings.
 *
 * @todo Assert mallocs
 */

/**
 * Implementation specific data types
 * (internal structure for hash collisions).
 */

/**
 * Hash table node structure.
 *
 */
struct hash_table_node{
    void * string_indexed;
};


/**
 * Hash table structure.
 *
 */
struct hash_table{
    int size;
    hash_table_node_t **fields;
    llist_t *str_keys;
};


/**
 * String indexed tree.
 */
typedef struct str_tree_node{
    struct str_tree_node * left;
    struct str_tree_node * right;
    ///int height;
    hash_t full_hash; /* is faster to check the hash (precalculated) than the
                       * entire string */

    //llist_node_t *key;
    char * index;
    void * value;

}str_tree_node_t;

/**
 *  Hashing code
 *  Returns the hash for a given variable using
 *  "One at a time hash".
 */
/**
 * Description: Returns the hash associated to an string.
 *
 * @param s The string to be hashed.
 *
 * @return The associated hash to s.
 *
 */
hash_t get_hash(char *s){
    hash_t h = 0;
    int i;

    for (i = 0; s[i] != '\0'; i++ ){
        h += s[i];
        h += ( h << 10 );
        h ^= ( h >> 6 );
    }

    h += ( h << 3 );
    h ^= ( h >> 11 );
    h += ( h << 15 );

    return h;
}


/* Hash table creation/freeing */
/**
 * Description: Returns a hash table.
 *
 * @return A hash table.
 *
 */
hash_table_t create_hash_table(){
    return create_hash_table_with_size(DEFAULT_HASH_TABLE_SIZE);
}


/**
 * Description: Returns a hash table with the specified size.
 *
 * @note Hash table sizes should be prime numbers for better performance.
 *
 * @return A hash table.
 *
 */
hash_table_t create_hash_table_with_size(int size){
    hash_table_t table = (hash_table_t) malloc(sizeof(struct hash_table));
    assert(table != NULL);

    table->size = size;
    table->fields = (hash_table_node_t **)
                          malloc(size * sizeof(hash_table_node_t *));

    assert(table->fields != NULL);

    if (NULL == 0){
        bzero(table->fields, sizeof(hash_table_node_t *) * size);
    }
    else{
        int i;
        for (i = 0;i < size;i++){
            table->fields[(unsigned int) i] = NULL;
        }
    }

    table->str_keys = create_llist();
    assert(table->str_keys != NULL);

    return table;
}


/**
 * Description: Frees a node from the internal string indexed
 * binary tree.
 *
 * @param node The binary tree node to be freed.
 * @param free_content_f A function to free the values (NULL for none).
 * @param free_str_index_f A function to free the index (NULL for none).
 *
 */
void free_str_index_tree(str_tree_node_t * node,
                         void (* free_content_f) (void *)){

    if (node != NULL){
        free_str_index_tree(node->right, free_content_f);
        free_str_index_tree(node->left,  free_content_f);

        free(node->index);

        if (free_content_f != NULL){
            free_content_f(node->value);
        }

        free(node);
    }
}


/**
 * Description: Frees a node from the hash table.
 *
 * @param node The binary tree node to be freed.
 * @param free_content_f A function to free the values (NULL for none).
 * @param free_str_index_f A function to free
 *                         the string indexes (NULL for none).
 *
 */
void free_hash_table_node(hash_table_node_t * node,
                          void (* free_content_f) (void *)){

    assert(node != NULL);


    free_str_index_tree((str_tree_node_t *)node->string_indexed,
                        free_content_f);

    free(node);
}


/**
 * Description: Frees a hash table.
 *
 * @param table The hash table to be freed.
 * @param free_content_f A function to free the values (NULL for none).
 * @param free_str_index_f A function to free.
 *                         the string indexes (NULL for none).
 *
 */
void free_hash_table(hash_table_t table,
                     void (* free_content_f) (void *)) {

    if(table == NULL){
        return ;
    }

    hash_table_node_t *node;
    int i;

    free_llist(table->str_keys, NULL);

    for (i = 0;i < table->size; i++){
        node = table->fields[(unsigned int) i];
        if (node != NULL){
            free_hash_table_node(node, free_content_f);
        }
    }
    free(table->fields);

    free(table);
}


/* Hash table manipulation */
/**
 * Description: Creates a hash table node.
 *
 * @param hash The hash asociated to the node.
 *
 * @return A pointer to a hash_table_node_t with the specified hash and
 *          NULLed 'left', 'right', 'integer_indexed', 'string_indexed'.
 *
 */
hash_table_node_t * create_hash_table_node(hash_t hash){

    hash_table_node_t * node = (hash_table_node_t*)
                                malloc(sizeof(hash_table_node_t));

    node->string_indexed  = NULL;

    return node;
}


/**
 * Description: Inserts a hash table node in a hash table or returns
 * the one with the associated hash.
 *
 * @param table The hash table to lookup in.
 * @param hash  The associated hash to the node.
 *
 * @return A pointer to a hash_table_node_t.
 *
 */
hash_table_node_t * _insert_hash_table_node(hash_table_t table, hash_t hash){

    hash_table_node_t * node = table->fields[(unsigned int) hash % table->size];

    if (node == NULL){
        node = table->fields[(unsigned int) hash % table->size] = create_hash_table_node(hash);
    }

    return node;
}


/**
 * Description: creates a string indexed binary tree node.
 *
 * @param s The node index.
 * @param v The node value.
 *
 * @return A pointer to the str_tree_node_t.
 *
 */
str_tree_node_t * _make_str_tree_node(char *s, void *v){
    str_tree_node_t * node = (str_tree_node_t *)
                                     malloc(sizeof(str_tree_node_t));

    node->left = NULL;
    node->right = NULL;
    node->full_hash = get_hash(s);
    ///node->height = 0;
    node->index = strdup(s);
    node->value = v;

    return node;
}


/**
 * Description: Inserts a value indexed by a string into the hash table.
 *
 * @param table The hash table to lookup in.
 * @param s     The table index.
 * @param v     The value to insert.
 *
 */
void insert_hash_table(hash_table_t table, char *s, void *v){

    int i, done = 0;
    hash_t hash = get_hash(s);

    hash_table_node_t * rnode =
                  _insert_hash_table_node(table, hash);

    str_tree_node_t * node = (str_tree_node_t *)
                                            rnode->string_indexed;

    if (node == NULL){
        node = _make_str_tree_node(s, v);

        rnode->string_indexed = node;

    }

    else{
        /** @todo refactor with preprocessor ? */
        while(!done){
            /* Position lookup */
            if (hash < node->full_hash){
                if (node->left == NULL){
                    node->left = _make_str_tree_node(s, v);
                    done = 1;
                }

                else{
                    node = node->left;
                }
            }

            else if (hash > node->full_hash){
                if (node->right == NULL){
                    node->right = _make_str_tree_node(s, v);
                    done = 1;
                }

                else{
                    node = node->right;
                }
            }

            else{
                i = strcmp(s, node->index);
                if (i < 0){
                    if (node->left == NULL){
                        node->left = _make_str_tree_node(s, v);
                        done = 1;
                    }

                    else{
                        node = node->left;
                    }
                }

                else if (i > 0){
                    if (node->right == NULL){
                        node->right = _make_str_tree_node(s, v);
                        done = 1;
                    }

                    else{
                        node = node->right;
                    }
                }

                else{
                    // Found, overwrite
                    node->value = v;
                    done = 1; /* <uncomment /> */
                }
            }
        }
    }

    add_to_llist(table->str_keys, s);
}


/**
 * Description: Obtains the hash table node associated with a hash.
 *
 * @param table The hash table to lookup in.
 * @param hash  The target hash.
 *
 * @return A pointer to the hash_table_node_t associated with the hash.
 *
 */
hash_table_node_t *_get_values_for_hash(hash_table_t table, hash_t hash){

    return table->fields[(unsigned int) hash % table->size];
}


/**
 * Description: Obtains the value associated with a string in the hash table.
 *
 * @param table The hash table to lookup in.
 * @param s     The key string.
 *
 * @return The inserted value or NULL if not found.
 *
 */
void *get_hash_table(hash_table_t table, char *s){
    if (s == NULL){
        return NULL;
    }

    hash_t hash = get_hash(s);
    hash_table_node_t * rnode = _get_values_for_hash(table, hash);

    if (rnode == NULL){
        return NULL;
    }
    if (rnode->string_indexed == NULL){
        return NULL;
    }

    str_tree_node_t *node = (str_tree_node_t*)
                                        rnode->string_indexed;

    int i = 1;

    while ((node != NULL) && (i != 0)){
        if (hash < node->full_hash){
            node = node->left;
        }

        else if (hash > node->full_hash){
            node = node->right;
        }

        else{
            i = strcmp(s, node->index);

            if (i < 0){
                node = node->left;
            }

            else if (i > 0){
                node = node->right;
            }
        }
    }

    if (node == NULL){
        return NULL;
    }

    return node->value;
}


/**
 * Description: Obtains string key values of a hash table.
 *
 * @param table The hash table to lookup in.
 *
 * @return The linked list of the string keys.
 *
 */
llist_t *get_hash_table_str_keys(hash_table_t table){
    assert(table != NULL);
    return (table->str_keys);
}


#endif
