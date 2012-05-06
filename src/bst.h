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

#ifndef BST_H
#define BST_H

/***********/
/* DEFINES */
/***********/

/*==========*/
/* typedefs */
/*==========*/

typedef struct bst bst;


/*************/
/* FUNCTIONS */
/*************/

/*============*/
/* management */
/*============*/

/* initialize bst */
bst *bst_init(void);

/* remove all nodes from tree; leaves an empty tree */
void bst_clear(bst *t, void (*callback)(void*));

/* free a bst */
void bst_free(bst *t, void (*callback)(void*));


/*=================*/
/* data operations */
/*=================*/

/*--------*/
/* insert */
/*--------*/

/* insert an item into the tree */
int bst_insert(bst *t, long key, void *data);


/*--------*/
/* delete */
/*--------*/

/* delete item with equal key from tree */
int bst_remove(bst *t, long key, void (*callback)(void*));


/*----------*/
/* retrieve */
/*----------*/

/* return non-zero if there's an item with equal key in tree */
int bst_contains(bst *t, long key);

/* get (first) item with equal key */
void *bst_get(bst *t, long key);

#endif
