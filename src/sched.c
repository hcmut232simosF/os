
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
	int i ;

	for (i = 0; i < MAX_PRIO; i++) {
		struct queue_t *queue = &mlq_ready_queue[i];
		queue->size = 0;
		// Cheap hack: storing the slot in each queue_t instance to keep it simple.
		queue->curr_slot = MAX_PRIO - i;
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
void mlq_reset_curr_slots(void) {
	for (int i = 0; i < MAX_PRIO; i++) {
		mlq_ready_queue[i].curr_slot = MAX_PRIO - i;
	}
}

/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t * get_mlq_proc(void) {
	/* TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 */
	struct pcb_t *ret = NULL;

	pthread_mutex_lock(&queue_lock);

	// Is nothing ready? Just return null.
	if (queue_empty() != -1) {
		pthread_mutex_unlock(&queue_lock);
		return NULL;
	}

	for (unsigned i = 0; i < MAX_PRIO; ++i) {
		struct queue_t *queue = &mlq_ready_queue[i];
		if (empty(queue)) {
			if (i == MAX_PRIO - 1) {
				mlq_reset_curr_slots();
			}
			continue;
		}

		if (queue->curr_slot > 0) {
			ret = dequeue(&mlq_ready_queue[i]);
			queue->curr_slot--;
			break;
		} else {
			if (i == MAX_PRIO - 1) {
				// Reset slots and start over.
				mlq_reset_curr_slots();
				i = 0;
			}
		}
	}

	pthread_mutex_unlock(&queue_lock);

	return ret;
}

void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t * proc) {
	return add_mlq_proc(proc);
}
#else
struct pcb_t * get_proc(void) {
	/* (done) TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	struct pcb_t * proc = NULL;

	pthread_mutex_lock(&queue_lock);
    if (empty(&ready_queue)) {
		// Just move everything back into the ready queue.
		while (!empty(&run_queue)) {
            enqueue(&ready_queue, dequeue(&run_queue));
        }
	}
	// Pop one.
    if (!empty(&ready_queue)) {
        proc = dequeue(&ready_queue);
    }
    pthread_mutex_unlock(&queue_lock);

	return proc;
}

void put_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);	
}
#endif


