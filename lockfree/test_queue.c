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

#define POOL_SIZE 12

static struct entry {
	struct lqueue_node node;
	int data;
} pool[POOL_SIZE];

static struct lqueue queue;

static volatile size_t success_count[12];

static noreturn void* test(void *arg)
{
	const int cpu = (uintptr_t)arg;
	bool unused;

	while (1) {
		struct lqueue_node *tmp;
		struct entry *entry;
		size_t loop_num;

		tmp = lqueue_dequeue(&queue, &unused);
		assert(tmp != NULL);

		entry = container_of(tmp, struct entry, node);
		assert(entry->data == 0);
		entry->data = 1;
		__asm__ volatile ("":"+m"(entry)::"memory");
		//loop_num = random() % 100;
		//for (volatile size_t i = 0; i < loop_num; ++i) {}
		__asm__ volatile ("":"+m"(entry)::"memory");
		assert(entry->data == 1);
		entry->data = 0;
		lqueue_enqueue(&queue, tmp);
		++success_count[cpu];
	}
}

static noreturn void *watch_dog(void *arg);

int main(void)
{
	pthread_t th;
	lqueue_init(&queue);

	for (size_t i = 0; i < POOL_SIZE; ++i)
		lqueue_enqueue(&queue, &pool[i].node);

	for (size_t i = 1; i < 12; ++i)
		assert(pthread_create(&th, NULL, test, (void *)(uintptr_t)i) == 0);

	assert(pthread_create(&th, NULL, watch_dog, NULL) == 0);
	test((void *)0);
	return 0;
}

static void *watch_dog(void *arg)
{
	size_t count[12];

	for (size_t i = 0; i < 12; ++i)
		count[i] = success_count[i];

	while (1) {
		sleep(3);
		for (size_t i = 0; i < 12; ++i) {
			assert(count[i] != success_count[i]);
			count[i] = success_count[i];
			printf("%zu %zu,", i, count[i]);
		}
		putchar('\n');
	}
}
