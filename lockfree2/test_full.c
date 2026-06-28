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

#define NR_THREADS 8
#define NR_ELEMENTS 7
#define NR_LQUEUES 3

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

static inline __attribute__((__always_inline__))
struct lqueue_node *element_to_node(void *const element)
{
	return &((struct element *)element)->node;
}
#define QNULL(qid) ((void *)(uintptr_t) \
			((uintptr_t)((qid + 1) * alignof(struct lqueue_node)) | (uintptr_t)(alignof(struct lqueue_node) - 1)) \
			)
#define GNULL ((void *)(uintptr_t)((NR_LQUEUES + 1) * alignof(struct lqueue_node)))

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

// return random in [0, size - 1]
static inline __attribute__((__always_inline__))
int32_t thread_random_avg(const int32_t size)
{
	int32_t max = (INT32_MAX / size) * size;
	int32_t ran;
	while (unlikely((ran = thread_random()) >= max))
		;
	return ran % size;
}

static inline size_t pick_min_queue(void)
{
	const int32_t ran = thread_random_avg((NR_LQUEUES << 10) + 5);
	if (likely(ran < (NR_LQUEUES << 10)))
		return ran >> 10;

	uintptr_t min_count = atomic_load_explicit(&queues[0].queue.first.count, memory_order_relaxed);
	size_t min_i = 0;
	for (size_t i = 1; i < NR_LQUEUES; ++i) {
		uintptr_t count = atomic_load_explicit(&queues[i].queue.first.count, memory_order_relaxed);
		if (count < min_count) {
			min_count = count;
			min_i = i;
		}
	}

	return min_i;
}

static noreturn void *test(void *arg)
{
	size_t count;
	char rand_statebufs[1 << 16];
	const uintptr_t elements_addr = (uintptr_t)&test_data.elements;
	int ret = initstate_r(rand(), rand_statebufs, sizeof(rand_statebufs), &rand_state);
	assert(ret == 0);

	while (1) {
		size_t qid = thread_random_avg(NR_LQUEUES);
		struct element *const e = lqueue_dequeue_ex(&queues[qid].queue, QNULL(qid), GNULL, &test_data, element_to_node).element;
		if (!e)
			continue;

		assert((uintptr_t)e >= elements_addr && (uintptr_t)e % alignof(struct element) == 0 && e - &test_data.elements[0] < NR_ELEMENTS);
		count = e->count;
		assert(count == shadow_data.e_count[e->id - ID_OFFSET].nr);

		// qid = thread_random_avg(NR_LQUEUES);
		qid = pick_min_queue();

		e->count = shadow_data.e_count[e->id - ID_OFFSET].nr = count + 1;

		lqueue_enqueue_ex(&queues[qid].queue, e, QNULL(qid), GNULL, &test_data, element_to_node);
		++shadow_data.t_count[(uintptr_t)arg].nr;

		if (unlikely((thread_random() & 0b111111) == 0)) {
			for (size_t i = 0; i < NR_LQUEUES; ++i)
				lqueue_free_sync(&queues[i].queue, GNULL);
		}
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
