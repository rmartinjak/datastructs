/* Copyright (c) 2012 Robin Martinjak.  All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
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

#include "hashtable.h"

#include <stdlib.h>
#include <errno.h>

/* structs */
struct hashtable {
	hash_t (*hash)(const void*, const void*);
	int (*cmp)(const void*, const void*, const void*);
	void (*free_key)(void*);
	void (*free_data)(void*);
	size_t n_items;
	size_t n_buckets;
	struct htbucket *buckets;
};

struct htiter {
	struct hashtable *ht;
	size_t b;
	struct htbucket_item *cur;
};

typedef struct htbucket {
	struct htbucket_item *root;
} htbucket;

struct htbucket_item {
	void *key;
	void *data;
	struct htbucket_item *next;
};


/* static prototypes */
static int ht_resize(hashtable *ht, int n);

static htbucket *ht_alloc_buckets(int n);
static int htbucket_empty(htbucket *b);
static int htbucket_insert(htbucket *b, void *key, void *data, int (*cmp)(const void*, const void*, const void*), const void *cmp_arg);
static void *htbucket_get(htbucket *b, const void *key, int (*cmp)(const void*, const void*, const void*), const void *cmp_arg);
static void *htbucket_remove(htbucket *b, const void *key, void (*free_key)(void*), int (*cmp)(const void*, const void*, const void*), const void *cmp_arg);
static void htbucket_pop(htbucket *b, void **key, void **data);
static void htbucket_clear(htbucket *b, void (*free_key)(void*), void (*free_data)(void*));


/* HASHTABLE FUNCTIONS */

static htbucket *ht_alloc_buckets(int n) {
	htbucket *ret;
	htbucket *p;

	if ((ret = malloc(n * sizeof(htbucket))) == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	p = ret;

	while (n--) {
		p->root = NULL;
		++p;
	}

	return ret;
}

int ht_init_f(hashtable **ht, hash_t (*hashfunc)(const void*, const void*),
	int (*cmpfunc)(const void*, const void*, const void*),
	void (*free_key)(void*), void (*free_data)(void*))
	{
	*ht = malloc(sizeof(hashtable));

	if (*ht == NULL) {
		errno = EINVAL;
		return HT_ERROR;
	}

	(*ht)->n_items = 0;
	(*ht)->n_buckets = HT_BUCKETS_MIN;

	(*ht)->hash = hashfunc;
	(*ht)->cmp = cmpfunc;

	(*ht)->free_key = free_key;
	(*ht)->free_data = free_data;

	(*ht)->buckets = ht_alloc_buckets((*ht)->n_buckets);

	if ((*ht)->buckets)
		return HT_OK;
	else {
		free(*ht);
		errno = ENOMEM;
		return HT_ERROR;
	}
}

#define FREE_KEY (free_key) ? free_key : ht->free_key
#define FREE_DATA (free_data) ? free_data : ht->free_data
void ht_free_f(hashtable *ht, void (*free_key)(void*), void (*free_data)(void*)) {
	int n = ht->n_buckets;

	while (n--) {
		htbucket_clear(ht->buckets + n, FREE_KEY, FREE_DATA);
	}
	free(ht->buckets);

	free(ht);
}
#undef FREE_KEY
#undef FREE_DATA

static int ht_resize(hashtable *ht, int n) {
	size_t i;
	hash_t k;
	htbucket *newbuckets;
	void *key, *data;

	if (n > HT_BUCKETS_MAX || n < HT_BUCKETS_MIN)
		return HT_OK;

	if ((newbuckets = ht_alloc_buckets(n)) == NULL) {
		errno = ENOMEM;
		return HT_ERROR;
	}

	for (i=0; i<ht->n_buckets; ++i) {
		while (1) {
			htbucket_pop(ht->buckets + i, &key, &data);
			/* no more items in bucket */
			if (!key)
				break;
			/* store in new bucket */
			k = (size_t)ht->hash(key, NULL) % n;

			/* meh, errors here are ignored
			 * (EINVAL won't happen and if ENOMEM occurs, you have other problems 
			 */
			htbucket_insert(newbuckets + k, key, data, ht->cmp, NULL);
		}
	}

	free(ht->buckets);
	ht->buckets = newbuckets;
	ht->n_buckets = n;

	return HT_OK;
}

#define FREE_DATA(p) (free_data) ? free_data(p) : ht->free_data(p)
int ht_set_fa(hashtable *ht, void *key, void *data,
	void (*free_key)(void*), void (*free_data)(void*),
	const void *hash_arg, const void *cmp_arg)
	{
	void *data_old;
	if (!free_data && !ht->free_data) {
		errno = EINVAL;
		return HT_ERROR;
	}
	data_old = ht_remove_fa(ht, key, free_key, hash_arg, cmp_arg);
	if (data_old) {
		FREE_DATA(data_old);
	}
	return ht_insert_a(ht, key, data, hash_arg, cmp_arg);
}
#undef FREE_DATA

int ht_insert_a(hashtable *ht, void *key, void *data,
	const void *hash_arg, const void *cmp_arg)
	{
	int res;
	hash_t k;

	if (!key || !data) {
		errno = EINVAL;
		return HT_ERROR;
	}

	k = (size_t)ht->hash(key, hash_arg) % ht->n_buckets;

	res = htbucket_insert(ht->buckets + k, key, data, ht->cmp, cmp_arg);

	if (res == HT_OK) {
		ht->n_items++;
		if (ht->n_items > ht->n_buckets)
			ht_resize(ht, ht->n_buckets << 1);
	}

	return res;
}

void *ht_get_a(hashtable *ht, const void *key,
	const void *hash_arg, const void *cmp_arg)
	{
	hash_t k;

	if (!ht || !key) {
		errno = EINVAL;
		return NULL;
	}
	k = (size_t)ht->hash(key, hash_arg) % ht->n_buckets;
	return htbucket_get(ht->buckets + k, key, ht->cmp, cmp_arg);
}

void *ht_remove_fa(hashtable *ht, const void *key,
	void (*free_key)(void*), const void *hash_arg, const void *cmp_arg)
	{
	hash_t k;
	void *res;

	if (!ht || !key || !free_key) {
		errno = EINVAL;
		return NULL;
	}

	k = (size_t)ht->hash(key, hash_arg) % ht->n_buckets;
	res = htbucket_remove(ht->buckets + k, key, free_key, ht->cmp, cmp_arg);

	if (res) {
		ht->n_items--;
		/* items < buckets/4 -> resize */
		if (ht->n_items * 4 < ht->n_buckets) {
			ht_resize(ht, ht->n_buckets >> 1);
		}
	}
	return res;
}

int ht_empty(hashtable *ht) {
	return (ht->n_items == 0);
}

void ht_pop(hashtable *ht, void **key, void **data) {
	size_t i;
	for (i = 0; i < ht->n_buckets; i++) {
		if (!htbucket_empty(ht->buckets + i)) {
			htbucket_pop(ht->buckets + i, key, data);
			return;
		}
	}
}


/* ITERATOR FUNCTIONS */

htiter *ht_iter(hashtable *ht) {
	htiter *it;

	if ((it = malloc(sizeof(htiter)))) {
		it->ht = ht;
		it->b = 0;
		it->cur = NULL;
	}
	return it;
}

int htiter_next(htiter *it, void **key, void **data) {
	if (it->cur && it->cur->next) {
		it->cur = it->cur->next;
	}
	else {
		/* not the first item */
		if (it->cur)
			it->b++;

		while (it->b < it->ht->n_buckets && htbucket_empty(it->ht->buckets + it->b))
			it->b++;

		if (it->b >= it->ht->n_buckets) {
			*key = NULL;
			*data = NULL;
			return 0;
		}

		it->cur = (it->ht->buckets + it->b)->root;
	}

	if (key)
		*key = it->cur->key;
	if (data)
		*data = it->cur->data;
	return 1;
}


/* BUCKET FUNCTIONS */

static int htbucket_empty(htbucket *b) {
	return (b->root == NULL);
}

/* insert item into bucket */
static int htbucket_insert(htbucket *b, void *key, void *data,
	int (*cmp)(const void*, const void*, const void*), const void *cmp_arg)
	{
	struct htbucket_item *p, *ins;

	if (!b || !key || !data || !cmp) {
		errno = EINVAL;
		return HT_ERROR;
	}

	/* initialize item */
	ins = malloc(sizeof(struct htbucket_item));
	if (!ins) {
		errno = ENOMEM;
		return HT_ERROR;
	}
	ins->key = key;
	ins->data = data;
	ins->next = NULL;

	/* empty htbucket */
	if (!b->root) {
		b->root = ins;
		return HT_OK;
	}
	/* compare with root */
	else if (cmp(key, b->root->key, cmp_arg) == 0) {
		free(ins);
		return HT_EXIST;
	}

	/* compare with child until p is last item */
	for (p = b->root; p->next; p = p->next) {
		if (cmp(key, p->next->key, cmp_arg) == 0) {
			free(ins);
			return HT_EXIST;
		}
	}

	p->next = ins;
	return HT_OK;
}

/* return data from bucket item with given key */
static void *htbucket_get(htbucket *b, const void *key, int (*cmp)(const void*, const void*, const void*), const void *cmp_arg) {
	struct htbucket_item *p;

	if (!b || !key || !cmp) {
		errno = EINVAL;
		return NULL;
	}

	for (p = b->root; p; p = p->next) {
		if (cmp(key, p->key, cmp_arg) == 0) {
			return p->data;
		}
	}
	return NULL;
}

/* remove and return item with the given key */
static void *htbucket_remove(htbucket *b, const void *key, void(*free_key)(void*), int (*cmp)(const void*, const void*, const void*), const void *cmp_arg) {
	struct htbucket_item *p, *del;
	void *ret;

	if (!b || !key || !free_key || !cmp) {
		errno = EINVAL;
		return NULL;
	}

	if (!b->root)
		return NULL;
	else if (cmp(key, b->root->key, cmp_arg) == 0) {
		del = b->root;
		b->root = del->next;
		ret = del->data;
		free_key(del);
		free(del);
		return ret;
	}

	for (p = b->root; p->next; p = p->next) {
		if (cmp(key, p->next->key, cmp_arg) == 0) {
			del = p->next;
			p->next = del->next;
			ret = del->data;
			free_key(del);
			free(del);
			return ret;
		}
	}
	return NULL;
}

/* remove first item from bucket, storing key and data in the passed pointers */
static void htbucket_pop(htbucket *b, void **key, void **data) {
	struct htbucket_item *p;

	if (!b->root) {
		*key = NULL;
		*data = NULL;
		return;
	}

	p = b->root;
	b->root = p->next;

	*key = p->key;
	*data = p->data;
	free(p);
}

/* remove all items from the bucket, using the passed free_*() functions on key and data */
static void htbucket_clear(htbucket *b, void (*free_key)(void*),  void (*free_data)(void*)) {
	struct htbucket_item *p, *del;

	if (!b) {
		errno = EINVAL;
		return;
	}

	p = b->root;

	while (p) {
		del = p;
		p = p->next;

		if (free_key) free_key(del->key);
		if (free_data) free_data(del->data);
		free(del);
	}
}
