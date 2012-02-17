Overview
========

types
~~~~~
hashtable
	hashtable object to operate on
htiter
	object to iterate through all items in a hashtable
hash_t
    return value of hash function, castable to size_t (atm: unsigned long)

functions and macros
~~~~~~~~~~~~~~~~~~~~

hashtable management
--------------------
initializing a hashtable::

	int ht_init(hashtable **ht,
			hash_t (*hashfunc)(const void*, const void*)
			int (*cmpfunc)(const void*, const void*, const void*);
	int ht_init_f(hashtable **ht,
			hash_t (*hashfunc)(const void*, const void*)
			int (*cmpfunc)(const void*, const void*, const void*),
			void (*free_key)(void*),
			void (*free_data)(void*);

free a hashtable and all keys/data::

	void ht_free(hashtable *ht);
	void ht_free_f(hashtable *ht,
			void (*free_key)(void*), void (*free_data)(void*));

determine if hashtable is empty::

	int ht_empty(hashtable *ht);


data operations
---------------
insert (not overwrite!) an item::

	int ht_insert(hashtable *ht, void *key, void *data);

	int ht_insert_a(hashtable *ht, void *key, void *data,
			const void *hash_arg, const void *cmp_arg);

overwrite existing / insert new item::

	int ht_set(hashtable *ht, void *key, void *data);

	int ht_set_a(hashtable *ht, void *key, void *data,
			const void *hash_arg, const void *cmp_arg);

	int ht_set_f(hashtable *ht, void *key, void *data,
			void (*free_key)(void*), void (*free_data)(void*));

	int ht_set_fa(hashtable *ht, void *key, void *data,
			void (*free_key)(void*), void (*free_data)(void*),
			const void *hash_arg, const void *cmp_arg);

retrieve stored data::

	void *ht_get(hashtable *ht, const void *key);
	void *ht_get_a(hashtable *ht, const void *key,
			const void *hash_arg, const void *cmp_arg);

remove and retrieve data::

	void *ht_remove(hashtable *ht, const void *key);
	void *ht_remove_a(hashtable *ht, const void *key,
			const void *hash_arg, const void *cmp_arg);
	void *ht_remove_f(hashtable *ht, const void *key,
			void (*free_key)(void*));
	void *ht_remove_fa(hashtable *ht, const void *key,
			void (*free_key)(void*),
			const void *hash_arg, const void *cmp_arg);

pop (retrieve & remove) the first item (first item in first non-empty bucket)::

	void ht_pop(hashtable *ht, void **key, void **data);

iteration
---------
create iteration object (can be free'd with free())::

	htiter *ht_iter(hashtable *ht);

retrieve next key and data (returns 0 if nothing retrieved)::

	int htiter_next(htiter *it, void **key, void **data);


Detailed description
====================

Creating a hashtable
--------------------

1. Define a hash function that takes two ``const void*`` arguments and returns
   an integer value of type ``hash_t``:
   - The first argument will be the *key* of a hashtable entry (-> the value
     to be hashed)
   - The second argument is passed when using one of the
   ``[...]_a()`` or ``[...]_fa()`` functions, else ``NULL``

2. Define a compare function that takes three ``const void*`` arguments and
   returns an ``int`` with value 0 if the first two arguments are
   *semantically equal* (like i.e. ``strcmp()``)
   - The first two argument will be two *keys* to compare
   - The third argument is, again, used with ``[...]_a()`` or ``[...]_fa()``
   	 (see above)

3. Define functions that take one *void** argument which will free *key* and
   *data* of your hashtable entries.

4. Declare and initialize a hashtable* object, using one of the following
   functions:
   - ``ht_init()`` which takes the following arguments:
      1. A pointer to your hashtable* object
	  2. Your hash function
	  3. Your compare function
   - ``ht_init_f()`` which takes the same arguemts as ``ht_init()`` plus
      4. Your function to free *keys*
	  5. Your function to free *data*
   If you use ``ht_init()`` you can still free keys and data by using the
   ``[...]_f()`` or ``[...]_fa()`` functions which also take freeing functions
   as arguments

Example::
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "hashtable.h"

	hash_t my_hash(const void *key, const void *arg);
	int my_cmp(const void *key1, const void *key2, const void *arg);

	hash_t my_hash(const void *key, const void *arg) {
		hash_t hash = 5381;
		const char *str = (const char*)key;
		size_t n;
		int c;

		if (!arg) {
			while ((c = *str++))
				hash = ((hash << 5) + hash) + c;
		}
		else {
			n = *(size_t*)arg;
			while ((c = *str++) && n-- > 0)
				hash = ((hash << 5) + hash) + c;
		}

		return hash;
	}

	int my_cmp(const void *key1, const void *key2, const void *arg) {
		size_t n;

		if (!arg) {
			return strcmp((const char*)key1, (const char*)key2);
		}
		else {
			n = *(size_t*)arg;
			return strncmp((const char*)key1, (const char*)key2, n);
		}
	}

	int main(int argc, char **argv) {
		hashtable *ht;

		ht_init_f(&ht, my_hash, my_cmp, free, free);

		[ ... ]

		return EXIT_SUCCESS;
	}
