#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>

#include <pthread.h>
#include <unistd.h>

#ifdef __WIN64__
#include <windows.h>
#elif defined(__linux__)
#include <sched.h>
#include <sys/mman.h>
#endif

#include "lqueue.h"

#ifndef container_of
#define container_of(ptr, type, member) ((type *)((uintptr_t)(ptr) - offsetof(type, member)))
#endif

#ifndef likely
#define likely(x)   __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#define XSTR(x) #x
#define STR(x) XSTR(x)

#define WRITE(out, cstr) \
	do { \
		write(out, cstr, sizeof(cstr) - 1); \
	} while (0)

#define ABORT_ON(x) \
	do { \
		if (unlikely(x)) { \
			WRITE(2, __FILE__ ":" STR(__LINE__) ":" #x "\n"); \
			abort(); \
		} \
	} while (0)

static inline unsigned long long gettime_ns(void)
{
	struct timespec ts;

	ABORT_ON(clock_gettime(CLOCK_MONOTONIC, &ts) != 0);

	return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static void bind_cpu(const int cpu)
{
#ifdef __WIN64__
	ABORT_ON(cpu < 0 || cpu >= sizeof(DWORD_PTR) * 8);

	ABORT_ON(SetThreadAffinityMask(GetCurrentThread(), (DWORD_PTR)1 << cpu) == 0);
#elif defined(__linux__)
	cpu_set_t mask;

	ABORT_ON(cpu < 0 || cpu >= CPU_SETSIZE);
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);

	ABORT_ON(sched_setaffinity(0, sizeof(mask), &mask));
#else
#error "unknown platfrom"
#endif
}

static inline uint64_t get_hw_tsc(void)
{
	register uint64_t rax __asm__("rax");
	register uint64_t rdx __asm__("rdx");
	__asm__ volatile ("rdtsc":"=r"(rax), "=r"(rdx)::);

	return (rdx << 32) | rax;
}

static inline void asm_loop(size_t loop)
{
	ABORT_ON(!__builtin_constant_p(loop) || loop == 0);
	
#ifdef __x86_64__
	__asm__ volatile (
			"1:decq	%0\n\t"
			"jne	1b"
			:"+r"(loop)
			:
			:"cc");
#else
#error "not support"
#endif
}

static void fill_random(void *const dst, const size_t n)
{
	for (size_t i = 0; i < n; i++)
		((char *)dst)[i] = rand();
}

#ifdef TEST_MUTEX
#elif defined(TEST_LQUEUE)
static struct lqueue g_lqueue;

struct element {
	struct lqueue_node node;
};

static inline void produce_element(struct element *const e)
{
	lqueue_enqueue(&g_lqueue, &e->node);
}
static inline bool consume_element(void)
{

	bool unused;
	struct lqueue_node *const node = lqueue_dequeue(&g_lqueue, &unused);

	return !!node;
}
#else
#error "test ?"
#endif

static int *g_p_cpus;
static int *g_c_cpus;
static size_t g_produce_count;
static size_t g_total_count;
static atomic_size_t g_nr_init_finished;
static atomic_bool g_start;
static atomic_size_t g_total_consumed;

static void *producer(void *const arg)
{
	const size_t count = g_produce_count;
	const size_t producer_id = (uintptr_t)arg;

	ABORT_ON(count > SIZE_MAX / sizeof(struct element));
	struct element *const nodes = malloc(sizeof(struct element) * count);

	bind_cpu(g_p_cpus[producer_id]);
	ABORT_ON(!nodes);

#ifndef __linux__
	// 触发缺页异常
	fill_random(nodes, sizeof(struct element) * count);
#endif

	printf("producer[%zu] init finished!\n", producer_id);
	++g_nr_init_finished;

	while (!g_start)
		;

	const unsigned long long start_time = gettime_ns();
	for (size_t i = 0; i < count; ++i)
		produce_element(&nodes[i]);
	const unsigned long long end_time = gettime_ns();

	printf("producer[%zu] finish use: %llu\n", producer_id, end_time - start_time);
	return NULL;
}

static void *consumer(void *const arg)
{
	const size_t total_count = g_total_count;
	const size_t consumer_id = (uintptr_t)arg;
	bind_cpu(g_c_cpus[consumer_id]);

	printf("consumer[%zu] init finished!\n", consumer_id);
	++g_nr_init_finished;

	while (!g_start)
		;
	
	const unsigned long long start_time = gettime_ns();
	while (1) {
		size_t success_count = 0;

		for (size_t i = 0; i < 1000; ++i) {
			if (consume_element())
				++success_count;
		}

		if (atomic_fetch_add(&g_total_consumed, success_count) + success_count == total_count)
			break;
	}
	const unsigned long long end_time = gettime_ns();

	printf("consumer[%zu] finish use: %llu\n", consumer_id, end_time - start_time);
	return NULL;
}


int main(void)
{
	size_t nr_p;
	size_t nr_c;
	int ret;
	pthread_t *p;
	pthread_t *c;

	srand(time(NULL));

	printf("input number of producers:");
	ret = scanf("%zu", &nr_p);
	ABORT_ON(ret != 1 || nr_p == 0 || nr_p > SIZE_MAX / sizeof(pthread_t));
	printf("input number of consumers:");
	ret = scanf("%zu", &nr_c);
	ABORT_ON(ret != 1 || nr_c == 0 || nr_c > SIZE_MAX / sizeof(pthread_t));

	p = malloc(sizeof(pthread_t) * nr_p);
	ABORT_ON(!p);
	c = malloc(sizeof(pthread_t) * nr_c);
	ABORT_ON(!c);
	g_p_cpus = malloc(sizeof(int) * nr_p);
	ABORT_ON(!g_p_cpus);
	g_c_cpus = malloc(sizeof(int) * nr_c);
	ABORT_ON(!g_c_cpus);

	printf("input counts from each producer:");
	ret = scanf("%zu", &g_produce_count);
	ABORT_ON(ret != 1);

	ABORT_ON(g_produce_count > SIZE_MAX / nr_p);
	g_total_count = g_produce_count * nr_p;

#ifdef __linux__
	ABORT_ON(mlockall(MCL_CURRENT | MCL_FUTURE));
#endif

#ifdef TEST_LQUEUE
	lqueue_init(&g_lqueue);
#endif

	for (size_t i = 0; i < nr_p; ++i) {
		printf("input producer[%zu] bind cpu:", i);
		ret = scanf("%d", &g_p_cpus[i]);
		ABORT_ON(ret != 1);
	}

	for (size_t i = 0; i < nr_c; ++i) {
		printf("input consumer[%zu] bind cpu:", i);
		ret = scanf("%d", &g_c_cpus[i]);
		ABORT_ON(ret != 1);
	}

	for (size_t i = 0; i < nr_p; ++i) {
		ret = pthread_create(&p[i], NULL, producer, (void *)(uintptr_t)i);
		ABORT_ON(ret);
	}

	for (size_t i = 0; i < nr_c; ++i) {
		ret = pthread_create(&c[i], NULL, consumer, (void *)(uintptr_t)i);
		ABORT_ON(ret);
	}

	while (g_nr_init_finished != nr_c + nr_p)
		;

	printf("sleep 3s and start to do test!\n");
	sleep(3);
	g_start = 1;

	for (size_t i = 0; i < nr_p; ++i)
		ABORT_ON(pthread_join(p[i], NULL));
	for (size_t i = 0; i < nr_c; ++i)
		ABORT_ON(pthread_join(c[i], NULL));

	printf("test finish!\n");
	return 0;
}
