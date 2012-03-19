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

#ifndef QUEUE_H
#define QUEUE_H

/***********/
/* DEFINES */
/***********/

/*==========*/
/* typedefs */
/*==========*/

typedef struct queue queue;


/*************/
/* FUNCTIONS */
/*************/

/*============*/
/* management */
/*============*/

/* initialize queue */
queue *q_init(void);

/* remove all items from queue, call callback() on them */
void q_clear(queue *q, void (*callback)(void*));

/* clear and free a queue */
void q_free(queue *q, void (*callback)(void*));

/* returns non-zero if there are no items in the queue */
int q_empty(queue *q);


/*=================*/
/* data operations */
/*=================*/

/* enqueue data as tail */
int q_enqueue(queue *q, void *data);

/* requeue data as head */
int q_requeue(queue *q, void *data);

/* dequeue head */
void *q_dequeue(queue *q);

/* check for existence using cmp() argument */
int q_contains(queue *q, const void *data, int(*cmp)(const void*, const void*));
int q_contains2(queue *q, const void *data, int(*cmp)(const void*, const void*, void*), void *arg);

#endif
