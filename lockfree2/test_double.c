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
#include <string.h>

#include "lqueue.h"

#define container_of(ptr, type, member) ((type *)((void *)(ptr) - offsetof(type, member)))

#define NR_THREADS 8
#define NR_ELEMENTS 6
#define NR_LQUEUES 2

#define ID_OFFSET ((size_t)0x1234aeffff)

struct element {
	size_t id __attribute__((aligned(256)));
	struct lqueue_node node;
	volatile size_t count;
};

struct __attribute__((aligned(4096))) {
	struct element elements[NR_ELEMENTS];
} test_data;

struct __attribute__((aligned(4096))) {
	struct lqueue queue __attribute__((aligned(256)));
} queues[NR_LQUEUES];

struct __attribute__((aligned(4096))) {
	struct __attribute__((aligned(256))) {
		volatile size_t nr;
	} e_count[NR_ELEMENTS];
	struct __attribute__((aligned(256))) {
		volatile size_t nr;
	} t_count[NR_THREADS];
} shadow_data;

#define GNULL NULL
#define QNULL(qid) ((void *)(uintptr_t)((uintptr_t)((qid + 1) * alignof(struct lqueue_node)) | 0b111))

static inline unsigned long long gettime_ns(void)
{
	struct timespec ts;

	assert(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);

	return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static __thread struct random_data rand_state;

static inline __attribute__((__always_inline__))
int32_t thread_random(void)
{
	int32_t result;
	const int ret = random_r(&rand_state, &result);
	assert(ret == 0);
	return result;
}

static inline size_t pick_min_queue(void)
{
	return (size_t)(atomic_load_explicit(&queues[0].queue.first.count, memory_order_relaxed) > atomic_load_explicit(&queues[1].queue.first.count, memory_order_relaxed));
}

static noreturn void *test(void *arg)
{
	size_t count;
	char rand_statebufs[1 << 16];
	const uintptr_t elements_addr = (uintptr_t)&test_data.elements;
	int ret = initstate_r(rand(), rand_statebufs, sizeof(rand_statebufs), &rand_state);
	assert(ret == 0);

	while (1) {
		const int32_t ran = thread_random();
		size_t qid = !!(ran & 0b1);
retry:
		struct lqueue_node *const node = lqueue_dequeue_ex(&queues[qid].queue, QNULL(qid), GNULL, NULL, default_element_to_node).element;
		if (!node) {
			qid = !qid;
			goto retry;
		}
		struct element *const e = container_of(node, struct element, node);

		assert((uintptr_t)e >= elements_addr && (uintptr_t)e % alignof(struct element) == 0 && e - &test_data.elements[0] < NR_ELEMENTS);
		count = e->count;
		assert(count == shadow_data.e_count[e->id - ID_OFFSET].nr);

		if (unlikely((ran & 0b111111111100) == 0))
			qid = pick_min_queue();
		else
			qid = !!(ran & 0b10);

		e->count = shadow_data.e_count[e->id - ID_OFFSET].nr = count + 1;

		lqueue_enqueue_ex(&queues[qid].queue, &e->node, QNULL(qid), GNULL, NULL, default_element_to_node);
		++shadow_data.t_count[(uintptr_t)arg].nr;
	}
}

static noreturn void *watch_dog(void *arg);

int main(void)
{
	pthread_t th;

	srand(gettime_ns());
	for (size_t i = 0; i < NR_LQUEUES; ++i)
		lqueue_init_ex(&queues[i].queue, GNULL);

	for (size_t i = 0; i < NR_ELEMENTS; ++i) {
		test_data.elements[i].id = i + ID_OFFSET;
#ifndef LQUEUE_NDEBUG
		memset(&test_data.elements[i].node, -1, sizeof(test_data.elements[i].node));
#endif
		lqueue_enqueue_ex(&queues[0].queue, &test_data.elements[i].node, QNULL(0), GNULL, (void *)0, default_element_to_node);
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
	size_t wd_q_count[NR_LQUEUES] = {};

	while (1) {
		sleep(8);
		printf("threads:");
		for (size_t i = 0; i < NR_THREADS; ++i) {
			assert(wd_t_count[i] != shadow_data.t_count[i].nr);
			wd_t_count[i] = shadow_data.t_count[i].nr;
			printf("%zu: %zu,", i, wd_t_count[i]);
		}
		printf("; nodes:");
		for (size_t i = 0; i < NR_ELEMENTS; ++i) {
			assert(wd_e_count[i] != shadow_data.e_count[i].nr);
			wd_e_count[i] = shadow_data.e_count[i].nr;
			printf("%zu: %zu,", i, wd_e_count[i]);
		}
		printf("; lqueues:");
		for (size_t i = 0; i < NR_LQUEUES; ++i) {
			uintptr_t count = atomic_load_explicit(&queues[i].queue.first.count, memory_order_relaxed);
			assert(wd_q_count[i] != count);
			wd_q_count[i] = count;
			printf("%zu: %zu,", i, wd_q_count[i]);
		}
		putchar('\n');
	}
}
