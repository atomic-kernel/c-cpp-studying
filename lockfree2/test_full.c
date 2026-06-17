#ifdef NDEBUG
#error "NDEBUG should not be defined!"
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "lqueue.h"

#define NR_THREADS 6
#define NR_ELEMENTS 10
#define NR_LQUEUES 3

struct element {
	size_t id __attribute__((aligned(128)));
	struct lqueue_node node;
	volatile size_t count;
};

struct {
	struct element elements[NR_ELEMENTS];
} test_data __attribute__((aligned(4096)));

struct {
	struct lqueue queue __attribute__((aligned(128)));
} queues[NR_LQUEUES] __attribute__((aligned(4096)));

volatile size_t e_count[NR_ELEMENTS];
volatile size_t t_count[NR_THREADS];

static inline __attribute__((__always_inline__))
struct lqueue_node *element_to_node(void *const element)
{
	return &((struct element *)element)->node;
}
#define GNULL ((void *)-(uintptr_t)((NR_LQUEUES + 1) * 16))
#define QNULL(qid) ((void *)-(uintptr_t)(((qid) + 1) * 16))

static inline size_t pick_queue(void)
{
	return (unsigned int)rand() % NR_LQUEUES;
}

static noreturn void* test(void *arg)
{
	size_t count;

	while (1) {
		size_t qid = pick_queue();
		struct element *const e = lqueue_dequeue_ex(&queues[qid].queue, QNULL(qid), GNULL, &test_data, element_to_node).element;
		if (!e)
			continue;

		count = e->count;
		assert(count == e_count[e->id]);
		e->count = e_count[e->id] = count + 1;

		qid = pick_queue();
		lqueue_enqueue_ex(&queues[qid].queue, e, QNULL(qid), GNULL, &test_data, element_to_node);
		++t_count[(uintptr_t)arg];
	}
}

static noreturn void *watch_dog(void *arg);

int main(void)
{
	pthread_t th;
	for (size_t i = 0; i < NR_LQUEUES; ++i)
		lqueue_init_ex(&queues[i].queue, GNULL);

	for (size_t i = 0; i < NR_ELEMENTS; ++i) {
		test_data.elements[i].id = i;
		lqueue_enqueue_ex(&queues[0].queue, &test_data.elements[i], QNULL(0), GNULL, &test_data, element_to_node);
	}

	for (size_t i = 1; i < NR_THREADS; ++i)
		assert(pthread_create(&th, NULL, test, (void *)(uintptr_t)i) == 0);

	assert(pthread_create(&th, NULL, watch_dog, NULL) == 0);
	test((void *)0);
	return 0;
}

static void *watch_dog(void *arg)
{
	size_t wd_t_count[NR_THREADS] = {};
	size_t wd_e_count[NR_ELEMENTS] = {};

	while (1) {
		sleep(8);
		printf("threads:");
		for (size_t i = 0; i < NR_THREADS; ++i) {
			assert(wd_t_count[i] != t_count[i]);
			wd_t_count[i] = t_count[i];
			printf("%zu: %zu,", i, wd_t_count[i]);
		}
		printf("; nodes:");
		for (size_t i = 0; i < NR_ELEMENTS; ++i) {
			assert(wd_e_count[i] != e_count[i]);
			wd_e_count[i] = e_count[i];
			printf("%zu: %zu,", i, wd_e_count[i]);
		}
		putchar('\n');
	}
}
