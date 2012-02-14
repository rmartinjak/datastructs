====================
hashtable.h defines:
====================


types
=====

::

hashtable	- hashtable object to operate on
htiter		- object to iterate through all items in a hashtable
hash_t		- return value of hash function, castable to size_t
			  (at the moment: unsigned long)

functions and macros
====================

hashtable management
--------------------
initializing a hashtable::

	int ht_init(hashtable **ht, hash_t (*hashfunc)(const void*, const void*)
				int (*cmpfunc)(const void*, const void*, const void*);

free a hashtable and all keys/data::

	void ht_free(hashtable *ht, void (*free_key)(void*), void (*free_data)(void*));

determine if hashtable is empty::

	int ht_empty(hashtable *ht);


data operations
---------------
insert (not overwrite!) an item::

	int ht_insert(hashtable *ht, void *key, void *data);
	int ht_insert2(hashtable *ht, void *key, void *data, const void *hash_arg, const void *cmp_arg);

overwrite existing / insert new item::

	int ht_set(hashtable *ht, void *key, void *data,
				void (*free_key)(void*), void (*free_data)(void*));
	int ht_set2(hashtable *ht, void *key, void *data,
				void (*free_key)(void*), void (*free_data)(void*),
				const void *hash_arg, const void *cmp_arg);

retrieve stored data::

	void *ht_get(hashtable *ht, const void *key);
	void *ht_get2(hashtable *ht, const void *key, const void *hash_arg, const void *cmp_arg);

remove and retrieve data::

	void *ht_remove(hashtable *ht, const void *key, void (*free_key)(void*));
	void *ht_remove2(hashtable *ht, const void *key, void (*free_key)(void*),
						const void *hash_arg, const void *cmp_arg);

pop (retrieve and remove) the first item (first item in first non-empty bucket)::

	void ht_pop(hashtable *ht, void **key, void **data);

iteration
---------
create iteration object (can be free'd with free())::

	htiter *ht_iter(hashtable *ht);

retrieve next key and data::

	int htiter_next(htiter *it, void **key, void **data);
