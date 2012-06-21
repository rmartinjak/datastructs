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

#define RED 0
#define BLACK 1
#define LEAF 2

#define IS_LEAF(n) ((n)->color == LEAF)
#define IS_BLACK(n) (IS_LEAF(n) || (n)->color == BLACK)
#define IS_RED(n) ((n)->color == RED)

#define IS_LEFT_CHILD(n) ((n) == (n)->parent->left)
#define IS_RIGHT_CHILD(n) ((n) == (n)->parent->right)

#define SIBLING(n) ((!(n) || !(n)->parent) ? NULL : ((IS_LEFT_CHILD(n)) ? (n)->parent->right : (n)->parent->left))
#define UNCLE(n) ((!(n) || !(n)->parent) ? NULL : SIBLING((n)->parent))
#define GRANDPARENT(n) ((n)->parent->parent)



/*=========*/
/* structs */
/*=========*/

/* node in a bst */
typedef struct bstnode bstnode;
struct bstnode
{
    long key;
    int color;
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
static bstnode *bst_findpath(bstnode *n, long key);

/* repair after inserting a node */
static void bst_insert_repair(bst *t, bstnode *n);

/* remove one/all nodes with equal key */
static void bst_remove_at(bst *t, bstnode *n);
/* repair after removing a node */
static void bst_remove_repair(bst *t, bstnode *n);

static void bst_rotate_left(bst *t, bstnode *n);
static void bst_rotate_right(bst *t, bstnode *n);


/********************/
/* STATIC FUNCTIONS */
/*******************/

static bstnode *bstnode_init(long key, void *data, bstnode *parent)
{
    bstnode *n, *l, *r;
    n = malloc(sizeof *n);
    l = malloc(sizeof *n);
    r = malloc(sizeof *n);

    if (!n || !l || !r)
    {
        free(n);
        free(l);
        free(r);
        return NULL;
    }

    n->key = key;
    n->color = RED;
    n->data = data;
    n->parent = parent;

    n->left = l;
    n->right = r;
    l->color = r->color = LEAF;
    l->key = r->key = -1;
    l->parent = r->parent = n;

    return n;
}

static void bstnode_free(bstnode *n, void (*callback)(void*))
{
    if (!n)
        return;

    if (!IS_LEAF(n))
    {
        bstnode_free(n->left, callback);
        bstnode_free(n->right, callback);

        if (callback)
            callback(n->data);
    }
    free(n);
}

static bstnode *bst_findpath(bstnode *n, long key)
{
    if (key == n->key)
        return n;
    else if (key < n->key)
        return (IS_LEAF(n->left)) ? n : bst_findpath(n->left, key);
    else
        return (IS_LEAF(n->right)) ? n : bst_findpath(n->right, key);
}

static void bst_insert_repair(bst *t, bstnode *n)
{
    bstnode *u;

    /* case 1: n is the root */
    if (!n->parent)
    {
        n->color = BLACK;
        return;
    }

    /* case 2: n's parent is black */
    if (n->parent->color == BLACK)
        return;

    /* case 3: n's uncle and father are both red */
    if ((u = UNCLE(n)) && u->color == RED)
    {
        n->parent->color = BLACK;
        u->color = BLACK;
        GRANDPARENT(n)->color = RED;
        bst_insert_repair(t, GRANDPARENT(n));
        return;
    }

    /* case 4: n has no or black uncle, red father and
        either  n = n->parent->right && n->parent = n->parent->parent->left
        or      n = n->parent->left  && n->parent = n->parent->parent->right
    */
    if (IS_RIGHT_CHILD(n) && IS_LEFT_CHILD(n->parent))
    {
        bst_rotate_left(t, n->parent);
        n = n->left;
    }
    else if (IS_LEFT_CHILD(n) && IS_RIGHT_CHILD(n->parent))
    {
        bst_rotate_right(t, n->parent);
        n = n->right;
    }

    /* case 5: n has no or black uncle, red father and
        either  n = n->parent->left  && n->parent = n->parent->parent->left
        or      n = n->parent->right && n->parent = n->parent->parent->right
    */
    n->parent->color = BLACK;
    GRANDPARENT(n)->color = RED;
    if (IS_LEFT_CHILD(n))
        bst_rotate_right(t, GRANDPARENT(n));
    else
    {
        bst_rotate_left(t, GRANDPARENT(n));
    }
}

static void bst_remove_at(bst *t, bstnode *n)
{
    bstnode *p;

    /* n has two children */
    if (!IS_LEAF(n->left) && !IS_LEAF(n->right))
    {
        static int del_from_left = 1;
        long tmpkey;
        void *tmpdata;
        if (del_from_left)
        {
            p = n->left;
            while (!IS_LEAF(p->right))
                p = p->right;
        }
        else
        {
            p = n->right;
            while (!IS_LEAF(p->left))
                p = p->left;
        }

        /* swap key and data and remove swapped node (which has 0 or 1 child) */
        tmpkey = p->key, p->key = n->key, n->key = tmpkey;
        tmpdata = p->data, p->data = n->data, n->data = tmpdata;
        bst_remove_at(t, p);
        return;
    }

    /* n has no/one child */
    p = IS_LEAF(n->left) ? n->right : n->left;

    /* replace n with p */
    p->parent = n->parent;
    if (n->parent)
    {
        if (IS_LEFT_CHILD(n))
            n->parent->left = p;
        else
            n->parent->right = p;
    }
    else
        t->root = p;

    if (IS_BLACK(n))
    {
        if (p->color == RED)
            p->color = BLACK;
        else
            bst_remove_repair(t, p);
    }
    free(n);
    return;
}

static void bst_remove_repair(bst *t, bstnode *n)
{
    bstnode *sib;

    /* case 1: n is root */
    if (!n->parent)
        return;

    sib = SIBLING(n);

    /* case 2: n's sibling is red */
    if (IS_RED(sib))
    {
        n->parent->color = RED;
        sib->color = BLACK;

        if (IS_LEFT_CHILD(n))
            bst_rotate_left(t, n->parent);
        else
            bst_rotate_right(t, n->parent);

        sib = SIBLING(n);
    }

    /* case 3: parent sib and sib's children are black */
    if (n->parent->color == BLACK &&
        sib->color == BLACK &&
        IS_BLACK(sib->left) &&
        IS_BLACK(sib->right))
    {
        sib->color = RED;
        bst_remove_repair(t, n->parent);
        return;
    }

    /* case 4: parent is red, sib and sib's children are black */
    if (n->parent->color == RED &&
        sib->color == BLACK &&
        IS_BLACK(sib->left) &&
        IS_BLACK(sib->right))
    {
        sib->color = RED;
        n->parent->color = BLACK;
        return;
    }

    /* case 5: 
        a) n is left child, sib and sib->right child are black, sib->left is red
        b) n is right child, sib and sib->left child are black, sib->right is red
    */
    if (IS_LEFT_CHILD(n) &&
        sib->color == BLACK &&
        IS_RED(sib->left) &&
        IS_BLACK(sib->right))
    {
        sib->color = RED;
        sib->left->color = BLACK;
        bst_rotate_right(t, sib);
        sib = SIBLING(n);
    }
    else if (IS_RIGHT_CHILD(n) &&
        sib->color == BLACK &&
        IS_RED(sib->right) && 
        IS_BLACK(sib->left))
    {
        sib->color = RED;
        sib->right->color = BLACK;
        bst_rotate_left(t, sib);
        sib = SIBLING(n);
    }

    /* case 6: */ 
    sib->color = n->parent->color;
    n->parent->color = BLACK;
    if (IS_LEFT_CHILD(n))
    {
        sib->right->color = BLACK;
        bst_rotate_left(t, n->parent);
    }
    else
    {
        sib->left->color = BLACK;
        bst_rotate_right(t, n->parent);
    }
}


#define ROTATE(dir, other)                          \
static void bst_rotate_##dir(bst *t, bstnode *n)    \
{                                                   \
    bstnode *p;                                     \
                                                    \
    if (!n || !n->other)                            \
        return;                                     \
                                                    \
    p = n->other;                                   \
                                                    \
    if (n == t->root)                               \
    {                                               \
        t->root = p;                                \
        p->parent = NULL;                           \
    }                                               \
    else                                            \
    {                                               \
        if (IS_LEFT_CHILD(n))                       \
            n->parent->left = p;                    \
        else                                        \
            n->parent->right = p;                   \
        p->parent = n->parent;                      \
    }                                               \
                                                    \
    n->other = p->dir;                              \
    if (n->other)                                   \
        n->other->parent = n;                       \
    p->dir = n;                                     \
    n->parent = p;                                  \
}

ROTATE(left, right)
ROTATE(right, left)

/**********************/
/* EXPORTED FUNCTIONS */
/**********************/

bst *bst_init(void)
{
    bst *t = malloc(sizeof *t);
    if (t)
    {
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
    bstnode *n;
    bstnode *ins;

    if (t->root && IS_LEAF(t->root))
    {
        bstnode_free(t->root, NULL);
        t->root = NULL;
    }

    if (!t->root)
    {
        t->root = bstnode_init(key, data, NULL);
        if (!t->root)
            return -1;

        t->root->color = BLACK;
        return 0;
    }

    n = bst_findpath(t->root, key);

    /* no duplicates allowed */
    if (key == n->key)
        return -1;

    ins = bstnode_init(key, data, n);
    if (!ins)
        return -1;

    /* new data smaller -> insert left */
    if (key < n->key)
    {
        free(n->left);
        n->left = ins;
    }
    /* new data greater -> insert right */
    else
    {
        free(n->right);
        n->right = ins;
    }

    /* repair the tree */
    bst_insert_repair(t, ins);

    return 0;
}

int bst_remove(bst *t, long key, void (*callback)(void*))
{
    bstnode *del;

    if (!t->root)
        return -1;

    del = bst_findpath(t->root, key);

    if (key != del->key)
        return -1;

    if (callback) callback(del->data);
    bst_remove_at(t, del);

    return 0;
}

int bst_contains(bst *t, long key)
{
    bstnode *n;

    if (!t || !t->root || IS_LEAF(t->root))
        return 0;

    n = bst_findpath(t->root, key);

    return (key == n->key);
}

void *bst_get(bst *t, long key)
{
    bstnode *n;

    if (!t || !t->root || IS_LEAF(t->root))
        return NULL;

    n = bst_findpath(t->root, key);

    if (key == n->key)
        return n->data;
    else
        return NULL;
}
