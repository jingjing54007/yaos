#include "types.h"

/* doubly linked list */

void LIST_LINK_INIT(struct list_t *list)
{
	list->next = list;
	list->prev = list;
}

void list_add(struct list_t *new, struct list_t *ref)
{
	new->prev = ref;
	new->next = ref->next;
	ref->next->prev = new;
	ref->next = new;
}

void list_del(struct list_t *item)
{
	item->prev->next = item->next;
	item->next->prev = item->prev;
}

/* fifo */

void fifo_init(struct fifo_t *q, void *queue, unsigned size)
{
	q->size  = size;
	q->buf   = queue;
	q->front = q->rear = 0;
}

void fifo_flush(struct fifo_t *q)
{
	q->front = q->rear = 0;
}

int fifo_get(struct fifo_t *q, int type_size)
{
	char *p = q->buf;
	int  v;

	if (q->front == q->rear) /* empty */
		return -1;

	v = *(int *)&p[q->front * type_size];

	q->front += 1;
	q->front %= q->size;

	return v;
}

int fifo_put(struct fifo_t *q, int value, int type_size)
{
	char *p = q->buf;
	register int i;

	if ( (q->rear+1) % q->size == q->front ) /* no more room */
		return -1;

	for (i = 0; i < type_size; i++) {
		p[q->rear * type_size + i] = value;
		value = (unsigned)value >> 8;
	}

	q->rear += 1;
	q->rear %= q->size;

	return value;
}