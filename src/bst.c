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

#include "bst.h"

#include <stdlib.h>


/***********/
/* DEFINES */
/***********/

/*========*/
/* macros */
/*========*/

#define BST_INTCMP(x, y) ((x < y) ? -1 : (x > y ? 1 : 0))
#define BST_CMP(t, x, y) ((t->cmp) ? t->cmp(x, y) : BST_INTCMP(x, y))


/*=========*/
/* structs */
/*=========*/

/* node in a bst */
typedef struct bstnode bstnode;
struct bstnode
{
    long key;
    void *data;
    bstnode *parent;
    bstnode *left;
    bstnode *right;
};

/* bst object, a pointer to it is the first argument to all bst_ functions */
struct bst
{
    int (*cmp)(long, long);
    bstnode *root;
};


/*===================*/
/* static prototypes */
/*===================*/

/* create a new bst node */
static bstnode *bstnode_init(long key, void *data, bstnode *parent);

/* free a bst node and all it's descendants recursively */
static void bstnode_free(bstnode *n, void (*callback)(void*));

/* find node in tree */
static bstnode *bst_findpath(bst *t, bstnode *n, long key);

/* insert a node */
static int bst_insert_(bst *t, long key, void *data, int dups);

/* find node that replaces a node n, move it to n's position and return it */
static bstnode *bst_remove_at(bstnode *n);

/* remove one/all nodes with equal key */
static int bst_remove_(bst *t, long key, void (*callback)(void*), int dups);


/********************/
/* STATIC FUNCTIONS */
/*******************/

static bstnode *bstnode_init(long key, void *data, bstnode *parent)
{
    bstnode *n = malloc(sizeof (bstnode));

    if (n) {
        n->key = key;
        n->data = data;
        n->parent = parent;
        n->left = NULL;
        n->right = NULL;
    }
    return n;
}

static void bstnode_free(bstnode *n, void (*callback)(void*))
{
    if (!n)
        return;

    bstnode_free(n->left, callback);
    bstnode_free(n->right, callback);

    if (callback)
        callback(n->data);
    free(n);
}

static bstnode *bst_findpath(bst *t, bstnode *n, long key)
{
    int cmp = BST_CMP(t, key, n->key);
    if (cmp == 0)
        return n;
    else if (cmp < 0)
        return (!n->left) ? n : bst_findpath(t, n->left, key);
    else
        return (!n->right) ? n : bst_findpath(t, n->right, key);
}

static int bst_insert_(bst *t, long key, void *data, int dups)
{
    bstnode *n;
    bstnode *ins;
    int cmp;

    if (!t->root) {
        t->root = bstnode_init(key, data, NULL);
        if (!t->root) {
            return -1;
        }
        return 0;
    }

    n = bst_findpath(t, t->root, key);

    cmp = BST_CMP(t, key, n->key);

    /* another node with equal data exists */
    if (cmp == 0) {
        /* no duplicates allowed */
        if (!dups) {
            return -1;
        }
        /* duplicates alloewd -> insert left */
        ins = bstnode_init(key, data, n);
        n->left = ins;
    }
    /* new data smaller -> insert left */
    else if (cmp < 0) {
        ins = bstnode_init(key, data, n);
        n->left = ins;
    }
    /* new data greater -> insert right */
    else {
        ins = bstnode_init(key, data, n);
        n->right = ins;
    }

    if (!ins) {
        return -1;
    }

    return 0;
}

static bstnode *bst_remove_at(bstnode *n)
{
    static int bst_del_from_left = 1;

    /* p will take n's position */
    bstnode *p;

    /* no children */
    if (!n->left && !n->right) {
        return NULL;
    }
    /* 1 child */
    else if (!n->left) {
        p = n->right;
    }
    else if (!n->right) {
        p = n->left;
    }

    /* 2 children */
    else {
        /* go either left or right way */
        bst_del_from_left ^= 1;

        /* n's predecessor replaces n */
        if (bst_del_from_left) {
            /* find predecessor (rightmost item of left subtree) */
            p = n->left;
            if (p->right) {
                while (p->right)
                    p = p->right;

                /* p's left subtree becomes his former parent's right subtree 
                   (p has no right subtree)
                 */
                if (p->left)
                    p->left->parent = p->parent;
                p->parent->right = p->left;

                /* p's new left subtree is n's subtree */
                p->left = n->left;
            }
            /* p's new left subtree is n's subtree */
            p->right = n->right;
        }
        /* analoguous to the above method */
        else {
            p = n->right;
            if (p->left) {
                while (p->left)
                    p = p->left;
                if (p->right)
                    p->right->parent = p->parent;
                p->parent->left = p->right;
                p->right = n->right;
            }
            p->left = n->left;
        }
    }

    /* update children's parent ptr */
    if (p->right)
        p->right->parent = p;
    if (p->left)
        p->left->parent = p;

    /* set parent */
    p->parent = n->parent;

    /* child ref of n's (now p's) parent update outside of this function */
    return p;
}

static int bst_remove_(bst *t, long key, void (*callback)(void*), int dups)
{
    /* node to remove and it's parent */
    bstnode *del, *par;

    /* number of removed nodes */
    int removed = 0;

    /* empty tree */
    if (!t->root) {
        return -1;
    }

    do {
        del = bst_findpath(t, t->root, key);

        /* no node found, return */
        if (BST_CMP(t, key, del->key) != 0) {
            return removed;
        }

        par = del->parent;

        /* bst_remove_at returns node that will replace the removed node
           uptdate del's parents (or t's root if del was the old root) */

        if (del == t->root)
            t->root = bst_remove_at(del);
        /* is left child */
        else if (del == par->left)
            par->left = bst_remove_at(del);
        /* is right child */
        else
            par->right = bst_remove_at(del);


        if (callback)
            callback(del->data);

        /* finally free it */
        free(del);

        removed++;
    /* repeat if dups non-zero until no node found */
    } while (dups);

    return removed;
}


/**********************/
/* EXPORTED FUNCTIONS */
/**********************/

bst *bst_init(int (*cmp)(long, long))
{
    bst *t = malloc(sizeof (bst));
    if (t) {
        t->cmp = cmp;
        t->root = NULL;
    }
    return t;
}

void bst_free(bst *t, void (*callback)(void*))
{
    bst_clear(t, callback);
    free(t);
}

void bst_clear(bst *t, void (*callback)(void*))
{
    if (!t || !t->root)
        return;

    bstnode_free(t->root, callback);
    t->root = NULL;
}

int bst_empty(bst *t)
{
    return (t->root == NULL);
}

int bst_insert(bst *t, long key, void *data)
{
    return bst_insert_(t, key, data, 0);
}

int bst_insert_dup(bst *t, long key, void *data)
{
    return bst_insert_(t, key, data, 1);
}

int bst_remove(bst *t, long key, void (*callback)(void*))
{
    return bst_remove_(t, key, callback, 0);
}

int bst_remove_dup(bst *t, long key, void (*callback)(void*))
{
    return bst_remove_(t, key, callback, 1);
}

int bst_contains(bst *t, long key)
{
    bstnode *n;

    if (!t || !t->root)
        return 0;

    n = bst_findpath(t, t->root, key);

    return (BST_CMP(t, key, n->key) == 0);
}

void *bst_get(bst *t, long key)
{
    bstnode *n;

    if (!t || !t->root)
        return 0;

    n = bst_findpath(t, t->root, key);

    if (BST_CMP(t, key, n->key) == 0)
        return n->data;
    else
        return NULL;
}
