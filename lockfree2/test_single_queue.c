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

#define POOL_SIZE 4
#define THREAD_NUM 4

static struct entry {
	struct lqueue_node node __attribute__((aligned(512)));
	size_t id;
	volatile size_t count;
} pool[POOL_SIZE];

static struct lqueue queue __attribute__((aligned(512)));

struct {
	volatile size_t count __attribute__ ((aligned (512)));
} shadow_count[POOL_SIZE];

static volatile size_t success_count[THREAD_NUM];

static noreturn void* test(void *arg)
{
	struct lqueue_node *tmp;
	struct entry *entry;
	size_t count;
	const uintptr_t thread_id = (uintptr_t)arg;

	while (1) {
		tmp = lqueue_dequeue(&queue).node;
		if (THREAD_NUM > POOL_SIZE && !tmp)
			continue;
		assert(tmp);

		entry = container_of(tmp, struct entry, node);
		count = entry->count;
		assert(count == shadow_count[entry->id].count);
		entry->count = shadow_count[entry->id].count = count + 1;

		lqueue_enqueue(&queue, tmp);
		++success_count[thread_id];
	}
}

static noreturn void *watch_dog(void *arg);

int main(void)
{
	pthread_t th;
	lqueue_init(&queue);

	for (size_t i = 0; i < POOL_SIZE; ++i) {
		pool[i].id = i;
		memset(&pool[i].node, -1, sizeof(pool[i].node));
		lqueue_enqueue(&queue, &pool[i].node);
	}

	for (size_t i = 1; i < THREAD_NUM; ++i)
		assert(pthread_create(&th, NULL, test, (void *)(uintptr_t)i) == 0);

	assert(pthread_create(&th, NULL, watch_dog, NULL) == 0);
	test((void *)0);
	return 0;
}

static void *watch_dog(void *arg)
{
	size_t count[THREAD_NUM] = {};
	size_t node_count[POOL_SIZE] = {};

	while (1) {
		sleep(8);
		printf("threads:");
		for (size_t i = 0; i < THREAD_NUM; ++i) {
			assert(count[i] != success_count[i]);
			count[i] = success_count[i];
			printf("%zu: %zu,", i, count[i]);
		}
		printf("; nodes:");
		for (size_t i = 0; i < POOL_SIZE; ++i) {
			assert(node_count[i] != pool[i].count);
			node_count[i] = pool[i].count;
			printf("%zu: %zu,", i, node_count[i]);
		}
		putchar('\n');
	}
}
