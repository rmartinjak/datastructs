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

#include "queue.h"

#include <stdlib.h>


/***********/
/* DEFINES */
/***********/

/*=========*/
/* structs */
/*=========*/

typedef struct qnode qnode;
struct qnode
{
    void *data;
    qnode *next;
};

struct queue
{
    qnode *head;
    qnode *tail;
};


/**********************/
/* EXPORTED FUNCTIONS */
/**********************/

queue *q_init(void)
{
    queue *q = malloc(sizeof (queue));

    if (q) {
        q->head = NULL;
        q->tail = NULL;
    }

    return q;
}

void q_clear(queue *q, void (*callback)(void*))
{
    void *del;
    while (q->head) {
        del = q_dequeue(q);
        if (callback)
            callback(del);
    }
}

void q_free(queue *q, void (*callback)(void*))
{
    q_clear(q, callback);
    free(q);
}

int q_enqueue(queue *q, void *data)
{
    qnode *ins;
    ins = malloc(sizeof (qnode));
    if (!ins) {
        return -1;
    }
    ins->data = data;
    ins->next = NULL;

    if (!q->head)
        q->head = ins;
    else
        q->tail->next = ins;

    q->tail = ins;
    return 0;
}

int q_requeue(queue *q, void *data)
{
    qnode *ins;
    ins = malloc(sizeof (qnode));
    if (!ins) {
        return -1;
    }
    ins->data = data;
    ins->next = q->head;

    q->head = ins;

    if (!q->tail)
        q->tail = ins;

    return 0;
}

void *q_dequeue(queue *q)
{
    void *ret;
    qnode *del;

    if (!q->head)
        return NULL;

    ret = q->head->data;
    del = q->head;

    q->head = q->head->next;

    if (!q->head)
        q->tail = NULL;

    free(del);

    return ret;
}

int q_empty(queue *q)
{
    return (q->head == NULL);
}

int q_contains(queue *q, const void *data, int(*cmp)(const void*, const void*))
{
    qnode *p;
    for (p = q->head; p; p = p->next) {
        if (cmp(data, p->data) == 0)
            return 1;
    }

    return 0;
}

int q_contains2(queue *q, const void *data, int(*cmp)(const void*, const void*, void*), void *arg)
{
    qnode *p;
    for (p = q->head; p; p = p->next) {
        if (cmp(data, p->data, arg) == 0)
            return 1;
    }

    return 0;
}
