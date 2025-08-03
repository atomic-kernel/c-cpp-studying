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
#include <stdatomic.h>
#include <string.h>

#include "lqueue.h"
#include "lstack.h"

#define container_of(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					\
	((type *)(__mptr - offsetof(type, member))); })

#define POOL_SIZE 10
#define THREAD_NUM 12

static struct entry {
	struct lqueue_node qnode;
	struct lstack_node snode;
	volatile int data;
	volatile size_t count;
} pool[POOL_SIZE];

static struct lqueue queue[2];
static struct lstack_head stack[2];

static volatile size_t success_count[THREAD_NUM];

static noreturn void* test(void *arg)
{
	const int cpu = (int)(uintptr_t)arg;
	bool unused;

	while (1) {
		struct entry *entry;
		unsigned int ran = rand() % 4;

		if (ran > 1) {
			struct lqueue_node *node;
			node = lqueue_dequeue(&queue[ran % 2], &unused);
			if (!node)
				continue;
			entry = container_of(node, struct entry, qnode);
		} else {
			struct lstack_node *node;
			node = lstack_pop(&stack[ran % 2], &unused);
			if (!node)
				continue;
			entry = container_of(node, struct entry, snode);
		}

		assert(entry->data == 0);
		entry->data = 1;
		++entry->count;
		__asm__ volatile ("":"+m"(entry)::"memory");
		assert(entry->data == 1);
		entry->data = 0;
		ran = rand() % 4;
		if (ran > 1)
			lqueue_enqueue(&queue[ran % 2], &entry->qnode);
		else
			lstack_push(&stack[ran % 2], &entry->snode);

		++success_count[cpu];
	}
	__builtin_unreachable();
}

static noreturn void *watch_dog(void *arg);

int main(void)
{
	pthread_t th;
	lqueue_init(&queue[0]);
	lqueue_init(&queue[1]);

	for (size_t i = 0; i < POOL_SIZE; ++i)
		lqueue_enqueue(&queue[0], &pool[i].qnode);

	for (size_t i = 1; i < THREAD_NUM; ++i)
		assert(pthread_create(&th, NULL, test, (void *)(uintptr_t)i) == 0);

	assert(pthread_create(&th, NULL, watch_dog, NULL) == 0);
	test((void *)0);
	__builtin_unreachable();
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
