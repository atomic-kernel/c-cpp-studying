#include <stdio.h>
#include <time.h>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "lqueue.h"
#include "lstack.h"


#define container_of(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					\
	((type *)(__mptr - offsetof(type, member))); })

#define SIZE ((size_t)671088640)

struct node {
	struct lqueue_node node;
	volatile int data;
} *nodes;
struct lqueue lqueue;

atomic_bool start = false;

static inline unsigned long long gettime_ns(void)
{
	struct timespec ts;

	assert(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);

	return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void *consume(void *arg)
{
	struct lqueue_node *node;
	size_t num = 0;
#ifdef __linux__
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(6, &mask);
	assert(sched_setaffinity(0, sizeof(cpu_set_t), &mask) == 0);
#endif

	while (!start) {}

	while (1) {
		node = __lqueue_dequeue(&lqueue);
		if (node) {
			__asm__ volatile ("# read":::"memory");
			container_of(node, struct node, node)->data;
			__asm__ volatile ("# read":::"memory");
			if (++num == SIZE)
				break;
		}
	}

	printf("consume time %llu\n", gettime_ns());

	return NULL;
}

void *produce(void *arg)
{
#ifdef __linux__
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(7, &mask);
	assert(sched_setaffinity(0, sizeof(cpu_set_t), &mask) == 0);
#endif

	while (!start) {}

	for (size_t i = 0; i < SIZE; ++i) {
		__asm__ volatile ("# write":::"memory");
		nodes[i].data = i;
		__asm__ volatile ("# write":::"memory");
		__lqueue_enqueue(&lqueue, &nodes[i].node);
	}

	printf("produce time %llu\n", gettime_ns());
	return NULL;
}


int main(int argc, char *argv[])
{
	assert(argc == 4);
	const long producer_num = atol(argv[1]);
	assert(producer_num > 0);
	const long consumer_num = atol(argv[2]);
	assert(consumer_num > 0);
	const long loop_time = atol(argv[3]);
	assert(loop_time > 0);
	nodes = malloc(sizeof(struct node) * SIZE);
	assert(nodes);

	struct lqueue_node node;

	pthread_t tc[consumer_num];
	pthread_t tp[producer_num];

	printf("size %zu\n", SIZE);
	__lqueue_init(&lqueue, &node);
	for (size_t i = 0; i < (size_t)consumer_num; ++i)
		assert(pthread_create(&tc[i], NULL, consume, (void *)(uintptr_t)loop_time) == 0);

	for (size_t i = 0; i < (size_t)producer_num; ++i)
		assert(pthread_create(&tp[i], NULL, produce, (void *)(uintptr_t)loop_time) == 0);

#ifdef __linux__
	assert(mlockall(MCL_CURRENT | MCL_FUTURE) == 0);
#endif
	sleep(8);

	// may be omit
	//memset(nodes, 1, sizeof(node) * SIZE);
	//memset(nodes, 101, sizeof(node) * SIZE);
	for (size_t i = 0; i < SIZE; ++i)
		nodes[i].data = i;
	printf("start time: %llu\n", gettime_ns());
	start = 1;

	while (1)
		sleep(10000);

	return 0;
}
