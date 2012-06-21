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

#include "hashtable.h"
#include "pgroups.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>


/***********/
/* DEFINES */
/***********/

/*========*/
/* macros */
/*========*/

#define FREE_KEY(p) if (free_key) free_key(p)
#define FREE_DATA(p) if (free_data) free_data(p)

#define RAND(min, max) ((rand() % (max)-(min)) + (min))
#define PRIME(group) (primes[group][RAND(0,PGROUP_ELEMENTS)])

/*=========*/
/* structs */
/*=========*/

struct hashtable
{
    hash_t (*hash)(const void*, const void*);
    int (*cmp)(const void*, const void*, const void*);
    void (*free_key)(void*);
    void (*free_data)(void*);
    size_t n_items;
    size_t n_buckets;
    size_t pgroup;
    struct htbucket *buckets;
};

struct htiter
{
    struct hashtable *ht;
    size_t b;
    struct htbucket_item *cur;
};

typedef struct htbucket
{
    struct htbucket_item *root;
} htbucket;

struct htbucket_item
{
    void *key;
    void *data;
    struct htbucket_item *next;
};


/*===================*/
/* static prototypes */
/*===================*/

/*---------------------------*/
/* internal management funcs */
/*---------------------------*/

static hash_t ht_strhash(const void *key, const void *arg);
static int ht_strcmp(const void *key1, const void *key2, const void *arg);

static int ht_resize(hashtable *ht, int grow);
static htbucket *ht_alloc_buckets(size_t n);


/*--------------*/
/* bucket funcs */
/*--------------*/

static int htbucket_empty(htbucket *b);

/* insert item into bucket */
static int htbucket_insert(htbucket *b, void *key, void *data,
        int (*cmp)(const void*, const void*, const void*), const void *cmp_arg);

/* return data from bucket item with given key */
static void *htbucket_get(htbucket *b, const void *key,
        int (*cmp)(const void*, const void*, const void*), const void *cmp_arg);

/* remove and return item with the given key */
static int htbucket_remove(htbucket *b, void **data, const void *key,
        int (*cmp)(const void*, const void*, const void*), const void *cmp_arg,
        void (*free_key)(void*));

/* remove first item from bucket, storing key and data in the passed pointers */
static void *htbucket_pop(htbucket *b, void **key, void **data);

/* remove all items from the bucket, using the passed free_*() functions on key and data */
static void htbucket_clear(htbucket *b,
        void (*free_key)(void*), void (*free_data)(void*));


/********************/
/* STATIC FUNCTIONS */
/********************/

/*==================*/
/* bucket functions */
/*==================*/

/* is the bucket empty? */
static int htbucket_empty(htbucket *b)
{
    return (b->root == NULL);
}

static int htbucket_insert(htbucket *b, void *key, void *data,
        int (*cmp)(const void*, const void*, const void*), const void *cmp_arg)
{
    struct htbucket_item *p, *ins;

    /* initialize item */
    ins = malloc(sizeof *ins);
    if (!ins)
        return HT_ERROR;

    ins->key = key;
    ins->data = data;
    ins->next = NULL;

    /* empty htbucket */
    if (!b->root)
    {
        b->root = ins;
        return HT_OK;
    }
    /* compare with root */
    else if (cmp(key, b->root->key, cmp_arg) == 0)
    {
        free(ins);
        return HT_EXIST;
    }

    /* compare with child until p is last item */
    for (p = b->root; p->next; p = p->next)
    {
        if (cmp(key, p->next->key, cmp_arg) == 0)
        {
            free(ins);
            return HT_EXIST;
        }
    }

    p->next = ins;
    return HT_OK;
}

static void *htbucket_get(htbucket *b, const void *key,
        int (*cmp)(const void*, const void*, const void*), const void *cmp_arg)
{
    struct htbucket_item *p;

    for (p = b->root; p; p = p->next)
    {
        if (cmp(key, p->key, cmp_arg) == 0)
            return p->data;
    }
    return NULL;
}

static int htbucket_remove(htbucket *b, void **data, const void *key,
        int (*cmp)(const void*, const void*, const void*), const void *cmp_arg,
        void (*free_key)(void*))
{
    struct htbucket_item *p, *del;

    if (!b->root)
        return HT_ERROR;
    else if (cmp(key, b->root->key, cmp_arg) == 0)
    {
        del = b->root;
        b->root = del->next;
        *data = del->data;
        FREE_KEY(del->key);
        free(del);
        return HT_OK;
    }

    for (p = b->root; p->next; p = p->next)
    {
        if (cmp(key, p->next->key, cmp_arg) == 0)
        {
            del = p->next;
            p->next = del->next;
            *data = del->data;
            FREE_KEY(del->key);
            free(del);
            return HT_OK;
        }
    }
    return HT_ERROR;
}

static void *htbucket_pop(htbucket *b, void **key, void **data)
{
    struct htbucket_item *p;

    if (!b->root)
    {
        *key = NULL;
        *data = NULL;
        return NULL;
    }

    p = b->root;
    b->root = p->next;

    *key = p->key;
    *data = p->data;
    free(p);

    return *data;
}

static void htbucket_clear(htbucket *b, void (*free_key)(void*),  void (*free_data)(void*))
{
    struct htbucket_item *p, *del;

    p = b->root;

    while (p)
    {
        del = p;
        p = p->next;

        FREE_KEY(del->key);
        FREE_DATA(del->data);
        free(del);
    }
}

/*==================================*/
/* "default" hash and cmp functions */
/*==================================*/

static hash_t ht_strhash(const void *key, const void *arg)
{
    unsigned char c;
    hash_t hash = 5381;
    const char *str = key;

    if (arg)
    {
        size_t n = *(size_t*)arg;
        while ((c = *str++) && n--)
            hash = ((hash << 5) + hash) + c;
    }
    else
    {
        while ((c = *str++))
            hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

static int ht_strcmp(const void *key1, const void *key2, const void *arg)
{
    const char *s1, *s2;
    s1 = key1;
    s2 = key2;

    if (arg)
    {
        size_t n = *(size_t*)arg;
        return strncmp(s1, s2, n);
    }
    else
        return strcmp(s1, s2);
}


/*===============================*/
/* internal management functions */
/*===============================*/

static htbucket *ht_alloc_buckets(size_t n)
{
    htbucket *ret;
    htbucket *p;

    ret = malloc(n * sizeof *ret);

    if (ret)
    {
        p = ret;
        while (n--)
            (p++)->root = NULL;
    }

    return ret;
}

static int ht_resize(hashtable *ht, int grow)
{
    size_t pg, n;
    size_t i;
    hash_t k;
    htbucket *newbuckets;
    void *key, *data;

    if (grow)
    {
        if (ht->pgroup >= PGROUP_COUNT-1)
            return HT_OK;
        pg = ht->pgroup+1;
    }
    else
    {
        if (ht->pgroup == 0)
            return HT_OK;
        pg = ht->pgroup-1;
    }

    n = PRIME(pg);

    if ((newbuckets = ht_alloc_buckets(n)) == NULL)
    {
        return HT_ERROR;
    }

    for (i=0; i<ht->n_buckets; ++i)
    {
        while (1)
        {
            htbucket_pop(ht->buckets + i, &key, &data);
            /* no more items in bucket */
            if (!key)
                break;
            /* store in new bucket */
            k = ht->hash(key, NULL) % n;

            /* meh, errors here are ignored
             * ENOMEM occurs, you have other problems
             */
            htbucket_insert(newbuckets + k, key, data, ht->cmp, NULL);
        }
    }

    free(ht->buckets);
    ht->buckets = newbuckets;
    ht->n_buckets = n;
    ht->pgroup = pg;

    return HT_OK;
}


/**********************/
/* EXPORTED FUNCTIONS */
/**********************/

/*=====================*/
/* hashtable functions */
/*=====================*/

void ht_statistics(hashtable *ht, int *n_items, int *n_buckets, int *empty, int *one, int *gtone, int *max, double *avg)
{
    int i;
    size_t n;
    struct htbucket_item *bi;

    *empty = *one = *gtone = *max = 0;
    *n_items = ht->n_items;
    *n_buckets = ht->n_buckets;

    for (i=0; i<ht->n_buckets; i++)
    {
        n = 0;
        for (bi = ht->buckets[i].root; bi; bi = bi->next)
            n++;

        switch (n) {
            case 0:
                (*empty)++;
                break;
            case 1:
                (*one)++;
                break;
            default:
                (*gtone)++;
                break;
        }
        if (n > *max)
            *max = n;
    }

    *avg = (double)ht->n_items / (ht->n_buckets - *empty);
}

/*------------*/
/* initialize */
/*------------*/

int ht_init_f(hashtable **ht, hash_t (*hashfunc)(const void*, const void*),
        int (*cmpfunc)(const void*, const void*, const void*),
        void (*free_key)(void*), void (*free_data)(void*))
{
    hashtable *p;

    /* if both func pointers are NULL use default string hash/cmp */
    if (!hashfunc && !cmpfunc)
    {
        hashfunc = ht_strhash;
        cmpfunc = ht_strcmp;
    }
    /* either both (see above) or none must be NULL */
    else if (!hashfunc || !cmpfunc)
    {
        *ht = NULL;
        return HT_ERROR;
    }

    p = malloc(sizeof *p);

    if (!p)
    {
        *ht = NULL;
        return HT_ERROR;
    }

    srand(time(NULL));

    p->n_items = 0;
    p->n_buckets = PRIME(0);
    p->pgroup = 0;

    p->hash = hashfunc;
    p->cmp = cmpfunc;

    p->free_key = free_key;
    p->free_data = free_data;

    p->buckets = ht_alloc_buckets(p->n_buckets);

    if (!p->buckets)
    {
        free(p);
        *ht = NULL;
        return HT_ERROR;
    }

    *ht = p;
    return HT_OK;
}


/*------*/
/* free */
/*------*/

void ht_free(hashtable *ht)
{
    if (!ht)
        return;

    ht_free_f(ht, ht->free_key, ht->free_data);
}

void ht_free_f(hashtable *ht, void (*free_key)(void*), void (*free_data)(void*))
{
    size_t n;

    if (!ht)
        return;

    n = ht->n_buckets;
    while (n--)
        htbucket_clear(ht->buckets + n, free_key, free_data);

    free(ht->buckets);

    free(ht);
}


/*-----*/
/* set */
/*-----*/

int ht_set_a(hashtable *ht, void *key, void *data,
        const void *hash_arg, const void *cmp_arg)
{
    if (!ht || !key || !data)
        return HT_ERROR;
    return ht_set_fa(ht, key, data, ht->free_key, ht->free_data, hash_arg, cmp_arg);
}

int ht_set_fa(hashtable *ht, void *key, void *data,
        void (*free_key)(void*), void (*free_data)(void*),
        const void *hash_arg, const void *cmp_arg)
{
    void *data_old;

    if (!ht || !key || !data)
        return HT_ERROR;

    data_old = ht_remove_fa(ht, key, free_key, hash_arg, cmp_arg);
    if (data_old)
    {
        FREE_DATA(data_old);
    }

    return ht_insert_a(ht, key, data, hash_arg, cmp_arg);
}


/*--------*/
/* insert */
/*--------*/

int ht_insert_a(hashtable *ht, void *key, void *data,
        const void *hash_arg, const void *cmp_arg)
{
    int res;
    hash_t k;

    if (!ht || !key)
        return HT_ERROR;

    k = ht->hash(key, hash_arg) % ht->n_buckets;

    res = htbucket_insert(ht->buckets + k, key, data, ht->cmp, cmp_arg);

    if (res == HT_OK)
    {
        ht->n_items++;
        if (ht->n_items > ht->n_buckets)
            ht_resize(ht, 1);
    }

    return res;
}


/*-----*/
/* get */
/*-----*/

void *ht_get_a(hashtable *ht, const void *key,
        const void *hash_arg, const void *cmp_arg)
{
    hash_t k;

    if (!ht || !key)
        return NULL;

    k = ht->hash(key, hash_arg) % ht->n_buckets;
    return htbucket_get(ht->buckets + k, key, ht->cmp, cmp_arg);
}


/*--------*/
/* remove */
/*--------*/

void *ht_remove_a(hashtable *ht, const void *key,
        const void *hash_arg, const void *cmp_arg)
{
    if (!ht || !key)
        return NULL;

    return ht_remove_fa(ht, key, ht->free_key, hash_arg, cmp_arg);
}

void *ht_remove_fa(hashtable *ht, const void *key,
        void (*free_key)(void*),
        const void *hash_arg, const void *cmp_arg)
{
    hash_t k;
    int res;
    void *data;

    if (!ht || !key)
        return NULL;

    k = ht->hash(key, hash_arg) % ht->n_buckets;
    res = htbucket_remove(ht->buckets + k, &data, key, ht->cmp, cmp_arg, free_key);

    if (res == HT_OK)
    {
        ht->n_items--;
        /* items < buckets/4 -> resize */
        if (ht->n_items * 4 < ht->n_buckets)
            ht_resize(ht, 0);
        return data;
    }

    return NULL;
}


/*-------*/
/* other */
/*-------*/

int ht_empty(hashtable *ht)
{
    if (!ht)
        return 1;

    return (ht->n_items == 0);
}

void *ht_pop(hashtable *ht, void **key, void **data)
{
    size_t i;

    if (!ht || !key || !data)
        return NULL;

    if (ht_empty(ht))
        return NULL;

    for (i = 0; i < ht->n_buckets; i++)
    {
        if (!htbucket_empty(ht->buckets + i))
            return htbucket_pop(ht->buckets + i, key, data);
    }
    return NULL;
}


/*====================*/
/* iterator functions */
/*====================*/

/* create iterator */
htiter *ht_iter(hashtable *ht)
{
    htiter *it = malloc(sizeof *it);

    if (it)
    {
        it->ht = ht;
        it->b = 0;
        it->cur = NULL;
    }
    return it;
}


/* get next key/data pair */
int htiter_next(htiter *it, void **key, void **data)
{
    /* still another item in current bucket */
    if (it->cur && it->cur->next)
        it->cur = it->cur->next;
    /* no more items in bucket */
    else
    {
        /* if !it->cur = fresh iterator, else use next bucket */
        if (it->cur)
            it->b++;

        /* skip empty buckets */
        while (it->b < it->ht->n_buckets && htbucket_empty(it->ht->buckets + it->b))
            it->b++;

        /* no more buckets! */
        if (it->b >= it->ht->n_buckets)
        {
            if (key) *key = NULL;
            if (data) *data = NULL;
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
