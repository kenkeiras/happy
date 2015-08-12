#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"
/**
 * @file hash_table.h
 *
 * @brief Ansi C hash table.
 * An ANSI C implementation of a hash table, which uses integers
 *  and '\0' ended strings as index.
 *
 */

/**
 * Data types.
 *
 */
/**
 * Hash (table index) datatype.
 *
 */
typedef int hash_t;


/**
 * Hash table node datatype.
 *
 */
struct hash_table_node;
typedef struct hash_table_node hash_table_node_t;


/**
 * Hash table datatype.
 *
 */
struct hash_table;
typedef struct hash_table *hash_table_t;


/**
 * @note Hash table size should be a prime number for better performance.
 *
 */
#ifndef DEFAULT_HASH_TABLE_SIZE
    #define DEFAULT_HASH_TABLE_SIZE 1021
#endif


/* Hashing */
/**
 * Description: Returns the hash associated to an string.
 *
 * @param s The string to be hashed.
 *
 * @return The associated hash to s.
 *
 */
hash_t get_hash(char *s);


/**
 * Hash table creation/freeing.
 *
 */
/**
 * Description: Returns a hash table with the default size.
 *
 * @return A hash table.
 *
 */
hash_table_t create_hash_table();


/**
 * Description: Returns a hash table with the specified size.
 *
 * @note Hash table sizes should be prime numbers for better performance.
 *
 * @return A hash table.
 *
 */
hash_table_t create_hash_table_with_size(int size);


/**
 * Description: Frees a hash table.
 *
 * @param table The hash table to be freed.
 * @param free_content_f A function to free the values (NULL for none).
 *
 */
void free_hash_table(hash_table_t table,
                     void (* free_content_f) (void *));


/**
 * Hash table manipulation.
 *
 */
/**
 * Description: Inserts a value indexed by a string into the hash table.
 *
 * @param table The hash table to lookup in.
 * @param s     The table index.
 * @param v     The value to insert.
 *
 */
void insert_hash_table(hash_table_t table, char *s, void *v);


/**
 * Description: Obtains the value associated with a string in the hash table.
 *
 * @param table The hash table to lookup in.
 * @param s     The key string.
 *
 * @return The inserted value or NULL if not found.
 *
 */
void *get_hash_table(hash_table_t table, char *s);


/**
 * Description: Obtains string key values of a hash table.
 *
 * @param table The hash table to lookup in.
 *
 * @return The linked list of the string keys.
 *
 */
llist_t *get_hash_table_str_keys(hash_table_t table);



#endif
