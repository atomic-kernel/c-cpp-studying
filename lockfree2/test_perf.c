#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdatomic.h>
#include <sys/mman.h>

#include "lqueue.h"

#define TH_NR_NODES 5000000

atomic_ullong start_time;

struct __attribute__((aligned(4096))) {
	struct lqueue queue;
	int pad __attribute__((aligned(512)));
	struct __attribute__((aligned(64))) {
		struct lqueue_node node;
		int id;
	} nodes[TH_NR_NODES * 6];
} test_data;

static inline unsigned long long gettime_ns(void)
{
	struct timespec ts;

	assert(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);

	return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void *func(void *arg)
{
	const uintptr_t id = (uintptr_t)arg;
	while (!start_time)
		;
	for (size_t i = id * TH_NR_NODES; i < (id + 1) * TH_NR_NODES; ++i) {
		// test_data.nodes[i].id = i;
		lqueue_enqueue(&test_data.queue, &test_data.nodes[i].node);
	}

	printf("%lu cost time %llu\n", id, gettime_ns() - start_time);

	return NULL;
}

int main()
{
	pthread_t th;
	int ret;

	ret = mlockall(MCL_CURRENT | MCL_FUTURE);
	assert(ret == 0);
	lqueue_init(&test_data.queue);

	for (size_t i = 1; i < 4; ++i) {
		ret = pthread_create(&th, NULL, func, (void *)(uintptr_t)i);
		assert(ret == 0);
	}

	sleep(1);
	asm_loop(1000000000);
	start_time = gettime_ns();
	asm_loop(2000);
	__asm__ volatile ("lfence");
	func((void *)(uintptr_t)0);

	while (1)
		sleep(1);

	return 0;
}
