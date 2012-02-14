/* Copyright (c) _arg01_arg Robin Martinjak.  All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.

   _arg. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>
#include <stdint.h>

/* return codes */
#define HT_OK 0
#define HT_ERROR -1
#define HT_EXIST 1

/* minimum and maximum number of buckets */
#ifndef HT_BUCKETS_MIN
	#define HT_BUCKETS_MIN (1 << 10)
#endif

#ifndef HT_BUCKETS_MAX
	#define HT_BUCKETS_MAX (1 << 16)
#endif

/* HT_BUCKETS_MAX may not be bigger than SIZE_MAX */
#if HT_BUCKETS_MAX > SIZE_MAX
	#undef HT_BUCKETS_MAX
	#define HT_BUCKETS_MAX SIZE_MAX
#endif

typedef struct hashtable hashtable;
typedef struct htiter htiter;
typedef unsigned int hash_t;


/* HASHTABLE FUNCTIONS */

#define ht_init(ht, hashfunc, cmpfunc) ht_init_free(ht, hashfunc, cmpfunc, NULL, NULL)
int ht_init_f(hashtable **ht, hash_t (*hashfunc)(const void*, const void*),
	int (*cmpfunc)(const void*, const void*, const void*),
	void (*free_key)(void*), void (*free_data)(void*));

#define ht_free(ht) ht_free_f(ht, NULL, NULL)
void ht_free_f(hashtable *ht, void (*free_key)(void*), void (*free_data)(void*));

int ht_empty(hashtable *ht);

#define ht_insert(ht, key, data) ht_insert_a(ht, key, data, NULL, NULL)
int ht_insert_a(hashtable *ht, void *key, void *data,
	const void *hash_arg, const void *cmp_arg);

#define ht_set(ht, key, data) ht_set_a(ht, key, data, NULL, NULL)
#define ht_set_a(ht, key, data, hash_arg, cmp_arg) ht_set_fa(ht, key, data, NULL, NULL, hash_arg, cmp_arg)
#define ht_set_f(ht, key, data, free_key, free_data) ht_set_fa(ht, key, data, free_key, free_data, NULL, NULL)
int ht_set_fa(hashtable *ht, void *key, void *data,
	void (*free_key)(void*), void (*free_data)(void*),
	const void *hash_arg, const void *cmp_arg);

#define ht_get(ht, key) ht_get_a(ht, key, NULL, NULL)
void *ht_get_a(hashtable *ht, const void *key,
	const void *hash_arg, const void *cmp_arg);

#define ht_remove(ht, key) ht_remove_a(ht, key, NULL, NULL)
#define ht_remove_a(ht, key, hash_arg, cmp_arg) ht_remove_fa(ht, key, NULL, hash_arg, cmp_arg)
#define ht_remove_f(ht, key, free_key) ht_remove_fa(ht, key, free_key, NULL, NULL)
void *ht_remove_fa(hashtable *ht, const void *key, void (*free_key)(void*),
	const void *hash_arg, const void *cmp_arg);

void ht_pop(hashtable *ht, void **key, void **data);

/* ITERATOR FUNCTIONS */
htiter *ht_iter(hashtable *ht);
int htiter_next(htiter *it, void **key, void **data);

#endif
