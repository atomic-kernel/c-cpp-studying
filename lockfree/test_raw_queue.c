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

#define container_of(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					\
	((type *)(__mptr - offsetof(type, member))); })

#define POOL_SIZE 3
#define THREAD_NUM 3

static struct entry {
	uint64_t pad[4096];
	struct lqueue_node node;
	uint64_t pad1[4096];
	int data;
	volatile size_t count;
} pool[POOL_SIZE];
static struct entry node_padding;

static struct raw_lqueue queue;

static volatile size_t success_count[THREAD_NUM];

static noreturn void* test(void *arg)
{
	const int cpu = (uintptr_t)arg;

	while (1) {
		struct lqueue_node *tmp;
		struct entry *entry;
		size_t loop_num;

		tmp = raw_lqueue_dequeue(&queue);
		if (THREAD_NUM > POOL_SIZE && !tmp)
			continue;
		assert(tmp);

		entry = container_of(tmp, struct entry, node);
		assert(entry->data == 0);
		entry->data = 1;
		__asm__ volatile ("":"+m"(entry)::"memory");
		assert(entry->data == 1);
		entry->data = 0;
		++entry->count;
		raw_lqueue_enqueue(&queue, tmp);
		++success_count[cpu];
	}
}

static noreturn void *watch_dog(void *arg);

int main(void)
{
	pthread_t th;
	raw_lqueue_init(&queue, &node_padding.node);

	for (size_t i = 0; i < POOL_SIZE; ++i)
		raw_lqueue_enqueue(&queue, &pool[i].node);

	for (size_t i = 1; i < THREAD_NUM; ++i)
		assert(pthread_create(&th, NULL, test, (void *)(uintptr_t)i) == 0);

	assert(pthread_create(&th, NULL, watch_dog, NULL) == 0);
	test((void *)0);
	return 0;
}

static void *watch_dog(void *arg)
{
	size_t count[THREAD_NUM];
	size_t node_count[POOL_SIZE];

	for (size_t i = 0; i < THREAD_NUM; ++i)
		count[i] = success_count[i];
	for (size_t i = 0; i < POOL_SIZE; ++i)
		node_count[i] = pool[i].count;

	while (1) {
		sleep(3);
		for (size_t i = 0; i < THREAD_NUM; ++i) {
			assert(count[i] != success_count[i]);
			count[i] = success_count[i];
			printf("%zu %zu,", i, count[i]);
		}
		for (size_t i = 0; i < POOL_SIZE; ++i) {
			assert(node_count[i] != pool[i].count);
			node_count[i] = pool[i].count;
			printf("%zu %zu,", i, node_count[i]);
		}
		putchar('\n');
	}
}
