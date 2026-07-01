#include <pthread.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <sched.h>
#include <unistd.h>

#ifndef LQUEUE_NDEBUG
#error "LQUEUE_NDEBUG should be defined"
#endif
#include "lqueue.h"

#define NR_GROUP_THREADS 2
#define STAT_COUNT 1000000000ULL

const static int target_cpus[2][NR_GROUP_THREADS] = {{11, 9}, {7, 5}};

struct lqueue q[2];

static void bind_cpu(const int cpu)
{
	int ret;
#ifdef __WIN64__
	assert(cpu >= 0 && cpu < sizeof(DWORD_PTR) * 8);

	ret = SetThreadAffinityMask(GetCurrentThread(), (DWORD_PTR)1 << cpu);
	assert(ret == 0);
#elif defined(__linux__)
	cpu_set_t mask;

	assert(cpu >= 0 && cpu < CPU_SETSIZE);
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);

	ret = sched_setaffinity(0, sizeof(mask), &mask);
	assert(ret == 0);
#else
#error "unknown platfrom"
#endif
}

static inline unsigned long long gettime_ns(void)
{
	struct timespec ts;

	assert(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);

	return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static void *thread_group0(void *arg)
{
	const uintptr_t thread_id = (uintptr_t)arg;
	size_t count = 0;
	unsigned long long start_time;
	
	bind_cpu(target_cpus[0][thread_id]);
	start_time = gettime_ns();
	while (1) {
		struct lqueue_node *const node = lqueue_dequeue(&q[0]).node;
		if (unlikely(!node)) {
			asm_loop(1 << 13);
			continue;
		}
		lqueue_enqueue(&q[1], node);
		if (unlikely(++count == STAT_COUNT)) {
			const unsigned long long tmp = gettime_ns();

			count = 0;
			printf("thread group 0, id %zu, count " STR(STAT_COUNT) "cost %lluns\n", thread_id, tmp - start_time);
			start_time = tmp;
		}
	}
}

static void *thread_group1(void *arg)
{
	const uintptr_t thread_id = (uintptr_t)arg;
	size_t count = 0;
	unsigned long long start_time;
	
	bind_cpu(target_cpus[1][thread_id]);
	start_time = gettime_ns();
	while (1) {
		struct lqueue_node *const node = lqueue_dequeue(&q[1]).node;
		if (unlikely(!node)) {
			asm_loop(1 << 13);
			continue;
		}
		lqueue_enqueue(&q[0], node);
		if (unlikely(++count == STAT_COUNT)) {
			const unsigned long long tmp = gettime_ns();

			count = 0;
			printf("thread group 1, id %zu, count " STR(STAT_COUNT) "cost %lluns\n", thread_id, tmp - start_time);
			start_time = tmp;
		}
	}
}

int main(void)
{
	pthread_t th;
	int ret;
	size_t size;
	size_t node_num = 0;

	printf("Want to use how many mem? (M):");
	ret = scanf("%zu", &size);
	assert(ret == 1);
	assert(size <= (SIZE_MAX >> 20));

	size <<= 20;
	void *const vaddr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_POPULATE, -1, 0);
	assert(vaddr != (void *)-1);
	ret = mlock(vaddr, size);
	assert(ret == 0);

	lqueue_init(&q[0]);
	lqueue_init(&q[1]);
	for (char *node = vaddr, *const end = vaddr + size;
			node + sizeof(struct lqueue_node) <= end;
			node += ((uintptr_t)(sizeof(struct lqueue_node) + CACHELINE_SIZE - 1) & (uintptr_t)-CACHELINE_SIZE)) {
		assert((uintptr_t)node % alignof(struct lqueue_node) == 0);
		lqueue_enqueue(&q[0], (struct lqueue_node *)node);
		++node_num;
	}
	printf("node num :%zu\n", node_num);

	for (size_t i = 0; i < NR_GROUP_THREADS; ++i) {
		ret = pthread_create(&th, NULL, thread_group0, (void *)(uintptr_t)i);
		assert(ret == 0);
	}
	for (size_t i = 0; i < NR_GROUP_THREADS; ++i) {
		ret = pthread_create(&th, NULL, thread_group1, (void *)(uintptr_t)i);
		assert(ret == 0);
	}
	while (1)
		pause();
	return 0;
}
