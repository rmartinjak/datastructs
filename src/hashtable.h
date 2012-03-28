/* Copyright (c) 2012 Robin Martinjak.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1.  Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

2.  Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    nd/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 *  \file hashtable.h
 *  \author Robin Martinjak
 *  \defgroup def     definitions
 *  \defgroup types   object types
 *  \defgroup mgmt    hash table management
 *  \defgroup dataop  data operation
 *  \defgroup iter    iteration
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>

/***********/
/* DEFINES */
/***********/

/*========*/
/* macros */
/*========*/

/*!
 *  \def        HT_OK
 *  \brief      returned on success
 *  \ingroup    def
 *
 *  \def        HT_ERROR
 *  \brief      returned on error
 *  \ingroup    def
 *
 *  \def        HT_EXIST
 *  \brief      returned by ht_insert_*() if key already present
 *  \ingroup    def
 */
#define HT_OK 0
#define HT_ERROR -1
#define HT_EXIST 1


/*==========*/
/* typedefs */
/*==========*/

/*! \brief      hash table instance
 *  \ingroup    types
 *
 *  \details
 *      A pointer to a hashtable object defines an instance of a
 *      hash table. This is usually the first argument to a function
 *      operating on a hash table.
 */
typedef struct hashtable hashtable;

/*! \brief      hash table iterator
 *  \ingroup    types
 *
 *  \details
 *      A pointer to a htiter object can be used to iterate throug
 *      all key/data pairs of a hashtable. It is obtained by calling
 *      ht_iter().
 */
typedef struct htiter htiter;

/*! \brief      hash value
 *  \ingroup    types
 *
 *  \details
 *      A ht_hashfunc_t function passed to ht_init() must return this type.
 */
typedef unsigned int hash_t;

/*! \brief      hash function
 *  \ingroup    types
 *
 *  \details
 *      A function that calculates a hash for a given key. The first argument
 *      is the key to be hashed, the second argument is optional (see i.e.
 *      ht_insert_a()) and might be NULL.
 */
typedef hash_t (*ht_hashfunc_t) (const void*, const void*);

/*! \brief      compare function
 *  \ingroup    types
 *
 *  \details
 *
 *      A function that compares two keys and returns zero if they are
 *      considered equal. The first two arguments are keys, the third
 *      argument is optional (see i.e. ht_insert_a()) and might be NULL.
 */
typedef int (*ht_cmpfunc_t) (const void*, const void*, const void*);


/*************/
/* FUNCTIONS */
/*************/

void ht_statistics(hashtable *ht, int *n_items, int *n_buckets, int *empty, int *one, int *gtone, int *max, double *avg);

/*===============*/
/* ht management */
/*===============*/

/*! \brief      initialize a \ref hashtable object
 *  \ingroup    mgmt
 *
 *  \details
 *      Initializes a hashtable WITHOUT functions to free keys and data.
 *
 *  \param      ht          pointer to a hashtable* object to be initialized
 *  \param      hashfunc    key hashing function
 *  \param      cmpfunc     key comparison function
 *
 *  \return     status code
 */
#define ht_init(ht, hashfunc, cmpfunc) ht_init_f(ht, hashfunc, cmpfunc, NULL, NULL)

/*! \brief      initialize a hashtable object
 *  \ingroup    mgmt
 *
 *  \details
 *      Initializes a hashtable WITH functions to free keys and data.
 *
 *  \param      ht          pointer to a hashtable* object to be initialized
 *  \param      hashfunc    key hashing function
 *  \param      cmpfunc     key comparison function
 *  \param      free_key    function to free keys (or NULL)
 *  \param      free_data   function to free data (or NULL)
 *
 *  \return     status code
 */
int ht_init_f(hashtable **ht,
        ht_hashfunc_t hashfunc, ht_cmpfunc_t cmpfunc,
        void (*free_key)(void*), void (*free_data)(void*));

/*! \brief      free a hashtable object
 *  \ingroup    mgmt
 *
 *  \details
 *      Frees a hashtable and all key/data pairs using the
 *      freeing functions specified to ht_init_f() earlier. If
 *      ht_init() was used or ht_init_f() with NULL as function pointer,
 *      keys and/or data will not be free'd.
 *
 *  \param  ht          hashtable* object
 */
void ht_free(hashtable *ht);

/*! \brief      Free a hashtable object
 *  \ingroup    mgmt
 *
 *  \details
 *      Frees a hashtable and all key/data pairs using the
 *      freeing functions passed as arguments. If NULL
 *      is passed, keys and/or data will not be free'd.
 *
 *  \param      ht          hashtable* object
 *  \param      free_key    function to free keys (or NULL)
 *  \param      free_data   function to free data (or NULL)
 */
void ht_free_f(hashtable *ht, void (*free_key)(void*), void (*free_data)(void*));

/*! \brief      Determine if hash table is empty
 *  \ingroup    mgmt
 *
 *  \return     Non-zero if hash table contains at least one key/data pair,
 *              zero otherwise
 */
int ht_empty(hashtable *ht);


/*================*/
/* data operation */
/*================*/

/*--------*/
/* insert */
/*--------*/
/*! \brief      Insert key/data pair
 *  \ingroup    dataop
 *
 *  \details
 *      Insert a key/data pair; if the key is already present, its
 *      corresponding data will NOT be overwritten.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *  \param      data        data
 *
 *  \return     HT_OK on success, HT_EXIST if a pair with
 *              equal key already exists, HT_ERROR if an error occured.
 */
#define ht_insert(ht, key, data) ht_insert_a(ht, key, data, NULL, NULL)

/*! \brief      Insert key/data pair with additional arguments
 *  \ingroup    dataop
 *
 *  \details
 *      Like ht_insert(), with additional argument to
 *      both hash and compare function.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *  \param      data        data
 *  \param      hash_arg    second argument to hash function
 *  \param      cmp_arg     third argument to compare function
 *
 *  \return     HT_OK on success, HT_EXIST if a pair with
 *              equal key already exists, HT_ERROR if an error occured.
 */
int ht_insert_a(hashtable *ht, void *key, void *data,
        const void *hash_arg, const void *cmp_arg);

/*-----*/
/* set */
/*-----*/
/*! \brief      Set key/data pair
 *  \ingroup    dataop
 *
 *  \details
 *      Set key/data pair, free()'s old key/data if existent
 *      using ht's freeing functions
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *  \param      data        data
 *
 *  \return     HT_OK on success,
 *              HT_ERROR if an error occured.
 */
#define ht_set(ht, key, data) ht_set_a(ht, key, data, NULL, NULL)

/*! \brief      Set key/data pair with additonal arguments
 *  \ingroup    dataop
 *
 *  \details
 *
 *      Like ht_set(), with additional argument to
 *      both hash and compare function.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *  \param      data        data
 *  \param      hash_arg    second argument to hash function
 *  \param      cmp_arg     third argument to compare function
 *
 *  \return     HT_OK on success,
 *              HT_ERROR if an error occured.
 */
int ht_set_a(hashtable *ht, void *key, void *data,
        const void *hash_arg, const void *cmp_arg);

/*! \brief      Set key/data pair with custom freeing funcs
 *  \ingroup    dataop
 *
 *  \details
 *
 *      Like ht_set_f(), with additional argument to
 *      both hash and compare function.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *  \param      data        data
 *  \param      free_key    function to free key
 *  \param      free_data   function to free data
 *
 *  \return     HT_OK on success,
 *              HT_ERROR if an error occured.
 */
#define ht_set_f(ht, key, data, free_key, free_data) ht_set_fa(ht, key, data, free_key, free_data, NULL, NULL)

/*! \brief      Set key/data pair with custom freeing funcs and additonal arguments
 *  \ingroup    dataop
 *
 *  \details
 *
 *      Like ht_set_f(), with additional argument to
 *      both hash and compare function.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *  \param      data        data
 *  \param      free_key    function to free key
 *  \param      free_data   function to free data
 *  \param      hash_arg    second argument to hash function
 *  \param      cmp_arg     third argument to compare function
 *
 *  \return     HT_OK on success,
 *              HT_ERROR if an error occured.
 */
int ht_set_fa(hashtable *ht, void *key, void *data,
        void (*free_key)(void*), void (*free_data)(void*),
        const void *hash_arg, const void *cmp_arg);

/*-----*/
/* get */
/*-----*/
/*! \brief      Retrieve data
 *  \ingroup    dataop
 *
 *  \details
 *      Retrieves the data associated with the given key.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *
 *  \return     data if exists or NULL
 */
#define ht_get(ht, key) ht_get_a(ht, key, NULL, NULL)

/*! \brief      Retrieve data with additional arguments
 *  \ingroup    dataop
 *
 *  \details
 *      Retrieves the data associated with the given key.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *  \param      hash_arg    second argument to hash function
 *  \param      cmp_arg     third argument to compare function
 *
 *  \return     data if exists or NULL
 */
void *ht_get_a(hashtable *ht, const void *key,
        const void *hash_arg, const void *cmp_arg);

/*--------*/
/* remove */
/*--------*/
/*! \brief      Remove and retrieve data
 *  \ingroup    dataop
 *
 *  \details
 *      Removes and retrieves the data associated with the given key.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *
 *  \return     data if exists or NULL
 */
#define ht_remove(ht, key) ht_remove_a(ht, key, NULL, NULL)

/*! \brief      Remove and retrieve data with additional arguments
 *  \ingroup    dataop
 *
 *  \details
 *      Removes and retrieves the data associated with the given key.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *  \param      hash_arg    second argument to hash function
 *  \param      cmp_arg     third argument to compare function
 *
 *  \return     data if exists or NULL
 */
void *ht_remove_a(hashtable *ht, const void *key,
        const void *hash_arg, const void *cmp_arg);

/*! \brief      Remove and retrieve data with custom key-freeing func
 *  \ingroup    dataop
 *
 *  \details
 *      Removes and retrieves the data associated with the given key;
 *      use custom key-freeing function.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *  \param      free_key    function to free key (or NULL)
 *
 *  \return     data if exists or NULL
 */
#define ht_remove_f(ht, key, free_key) ht_remove_fa(ht, key, free_key, NULL, NULL)

/*! \brief      Remove and retrieve data with custom key-freeing func and additional arguments
 *  \ingroup    dataop
 *
 *  \details
 *      Removes and retrieves the data associated with the given key;
 *      use custom key-freeing function.
 *
 *  \param      ht          hashtable* object
 *  \param      key         key
 *  \param      free_key    function to free key (or NULL)
 *  \param      hash_arg    second argument to hash function
 *  \param      cmp_arg     third argument to compare function
 *
 *  \return     data if exists or NULL
 */
void *ht_remove_fa(hashtable *ht, const void *key,
        void (*free_key)(void*),
        const void *hash_arg, const void *cmp_arg);

/*! \brief      Pop first item from first non-empty bucket
 *  \ingroup    dataop
 *
 *  \details
 *      Stores the first item from the first non-empty bucket
 *      in the given buffers
 *
 *  \param      ht          hashtable* object
 *  \param      key         buffer to store key in
 *  \param      data        buffer to store data in
 */
void *ht_pop(hashtable *ht, void **key, void **data);


/*===========*/
/* iteration */
/*===========*/

/*! \brief      Create iterator
 *  \ingroup    iter
 *
 *  \details
 *      Returns an object to iterate throug all key/data pairs
 *      of a hashtable. After use, the iterator should be free'd
 *      by using free().  *
 *  \param      ht          hashtable* object
 *
 *  \return     iterator instance
 */
htiter *ht_iter(hashtable *ht);

/*! \brief      Get next key/value pair
 *  \ingroup    iter
 *
 *  \details
 *      Get next key/value pair and store them in the given
 *      buffers. Stores NULL in each if no more items to iterate
 *
 *  \param      it          htiter* object
 *  \param      key         buffer to store key in
 *  \param      data        buffer to store data in
 *
 *  \return     zero if no more key/data pair retrieved, non-zero else
 */
int htiter_next(htiter *it, void **key, void **data);
#endif
