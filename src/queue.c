#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* (done) TODO: put a new process to queue [q] */

        if (q->size >= MAX_QUEUE_SIZE) {
                return;
        }

        q->proc[q->size] = proc;
        q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* (done) TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */

        struct pcb_t *ret = NULL;
        unsigned ret_index;
        for (unsigned i = 0; i < q->size; i++) {
                struct pcb_t *j = q->proc[i];
                bool cond;
#ifdef MLQ_SCHED
                cond = ret == NULL || j->prio > ret->prio;
#else
                cond = ret == NULL || j->priority > ret->priority;
#endif
                if (cond) {
                        ret = j;
                        ret_index = i;
                }
        }

        if (ret != NULL) {
                unsigned last = q->size - 1;
                // Still works if last == ret_index.
                q->proc[ret_index] = q->proc[last];
                q->proc[last] = NULL;
                q->size--;
        }

	return ret;
}

