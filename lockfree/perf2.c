#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <inttypes.h>

#include <pthread.h>
#include <unistd.h>

#ifdef __WIN64__
#include <windows.h>
#elif defined(__linux__)
#include <sched.h>
#include <sys/mman.h>
#endif

#include "lqueue.h"
#include "lstack.h"

#ifndef container_of
#define container_of(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					\
	((type *)(__mptr - offsetof(type, member))); })
#endif

#define HANDLE_CYCLES 200

#define TEST_MEM_SIZE (22ULL * 1024 * 1024 * 1024)
#define TEST_ELEMENT_NUM (TEST_MEM_SIZE / sizeof(struct element))

static atomic_bool start_test;

static _Atomic(uint64_t) producer_insert_total_time;
static _Atomic(uint64_t) consumer_delete_total_time;
static atomic_size_t total_consumed;
static atomic_size_t buf_elements_num;

static struct lstack_head element_pool;
struct element {
	struct lstack_node alloc_node;
	struct lqueue_node test_node;
};

static inline unsigned long long gettime_ns(void)
{
	struct timespec ts;

	assert(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);

	return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static void bind_cpu(const int cpu)
{
#ifdef __WIN64__
	assert(cpu > 0);
	assert(cpu < 64);

	assert(SetThreadAffinityMask(GetCurrentThread(), (1 << cpu)) != 0);
#elif defined(__linux__)
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);

	assert(sched_setaffinity(0, sizeof(mask), &mask) == 0);
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
	__asm__ volatile ("1:\n\t"
			"decq	%0\n\t"
			"jne	1b"
			:"+r"(loop)
			:
			:"cc");
}

static inline struct element *produce_element(void)
{
	bool unused;
	struct lstack_node *const node = lstack_pop(&element_pool, &unused);
	if (!node)
		return NULL;

	return container_of(node, struct element, alloc_node);
}

#ifdef TEST_MUTEX
static pthread_mutex_t test_lock = PTHREAD_MUTEX_INITIALIZER;
#define TEST_BUF_SIZE (3ULL * 8 * 1024 * 1024)
static struct element *volatile test_buf[TEST_BUF_SIZE];
static size_t test_buf_size;
static inline void insert_element(struct element *const e)
{
	assert(pthread_mutex_lock(&test_lock) == 0);
	assert(test_buf_size < TEST_BUF_SIZE);
	test_buf[test_buf_size++] = e;
	assert(pthread_mutex_unlock(&test_lock) == 0);
}
static inline size_t delete_element(void)
{
	assert(pthread_mutex_lock(&test_lock) == 0);
	assert(test_buf_size != 0);
	test_buf[--test_buf_size];
	assert(pthread_mutex_unlock(&test_lock) == 0);
	return 1;
}
#elif defined(TEST_LQUEUE)
static struct lqueue test_queue;
static inline void insert_element(struct element *const e)
{
	lqueue_enqueue(&test_queue, &e->test_node);
}
static inline size_t delete_element(void)
{

	bool unused;
	struct lqueue_node *const node = lqueue_dequeue(&test_queue, &unused);
	assert(node);
	return 1;
}
#endif

static void *producer(void *const arg)
{
	bind_cpu((uintptr_t)arg * 2 + 1);

	while (!atomic_load_explicit(&start_test, memory_order_acquire))
		;

	while (1) {
		struct element *const e = produce_element();
		if (unlikely(!e))
			break;

		asm_loop(HANDLE_CYCLES);

		const uint64_t start_hw_tsc = get_hw_tsc();
		insert_element(e);
		const uint64_t end_hw_tsc = get_hw_tsc();
		atomic_fetch_add_explicit(&buf_elements_num, 1, memory_order_release);

		if (likely(end_hw_tsc > start_hw_tsc))
			atomic_fetch_add_explicit(&producer_insert_total_time, end_hw_tsc - start_hw_tsc, memory_order_relaxed);
	}
	printf("producer end time %llu\n", gettime_ns());

	return NULL;
}

static void *consumer(void *const arg)
{
	bind_cpu((uintptr_t)arg * 2 + 7);

	while (!atomic_load_explicit(&start_test, memory_order_acquire))
		;

	while (1) {

		while (1) {
			size_t tmp = atomic_load_explicit(&buf_elements_num, memory_order_relaxed);

retry:
			if (tmp == 0) {
				if (unlikely(atomic_load_explicit(&total_consumed, memory_order_relaxed) == TEST_ELEMENT_NUM))
					goto out;
				continue;
			}
			if (atomic_compare_exchange_weak_explicit(&buf_elements_num, &tmp, tmp - 1, memory_order_acquire, memory_order_relaxed))
				break;
			goto retry;
		}
		const uint64_t start_hw_tsc = get_hw_tsc();
		const size_t num = delete_element();
		const uint64_t end_hw_tsc = get_hw_tsc();
		atomic_fetch_add_explicit(&total_consumed, num, memory_order_relaxed);
		if (likely(end_hw_tsc > start_hw_tsc))
			atomic_fetch_add_explicit(&consumer_delete_total_time, end_hw_tsc - start_hw_tsc, memory_order_relaxed);

		asm_loop(HANDLE_CYCLES);
	}
out:
	printf("comsumer end time %llu\n", gettime_ns());

	return NULL;
}

int main(void)
{
	pthread_t p[3];
	pthread_t c[3];
	struct element *const e = malloc(TEST_ELEMENT_NUM * sizeof(struct element));
	assert(e);

	for (size_t i = 0; i < 3; ++i)
		assert(pthread_create(&p[i], NULL, producer, (void *)(uintptr_t)i) == 0);

	for (size_t i = 0; i < 3; ++i)
		assert(pthread_create(&c[i], NULL, consumer, (void *)(uintptr_t)i) == 0);

	printf("press enter to start test...");
	scanf("%*[^\n]");
	scanf("%*c");

	for (size_t i = 0; i < TEST_ELEMENT_NUM; ++i)
		lstack_push(&element_pool, &e[i].alloc_node);
#ifdef TEST_LQUEUE
	lqueue_init(&test_queue);
#endif
#ifdef __linux__
	assert(mlockall(MCL_CURRENT | MCL_FUTURE) == 0);
#else
#ifdef TEST_MUTEX
	for (size_t i = 0; i < TEST_BUF_SIZE; ++i)
		test_buf[i] = (void *)(uintptr_t)i;
#endif
#endif
	start_test = true;

	for (size_t i = 0; i < 3; ++i)
		assert(pthread_join(p[i], NULL) == 0);
	for (size_t i = 0; i < 3; ++i)
		assert(pthread_join(c[i], NULL) == 0);

	printf("producer insert total time %" PRIu64  "\n", atomic_load_explicit(&producer_insert_total_time, memory_order_relaxed));
	printf("consumer delete total time %" PRIu64  "\n", atomic_load_explicit(&consumer_delete_total_time, memory_order_relaxed));
	printf("test finish!\n");
	return 0;
}
