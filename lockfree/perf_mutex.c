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
#include <stdint.h>
#include <stdatomic.h>

#define SIZE ((size_t)671088640)

static volatile int *buf;
static size_t buf_size = 0;
static pthread_mutex_t lock;

static atomic_bool start = false;

static inline unsigned long long gettime_ns(void)
{
	struct timespec ts;

	assert(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);

	return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void *consume(void *arg)
{
	size_t num = 0;
#ifdef __linux__
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(6, &mask);
	assert(sched_setaffinity(0, sizeof(cpu_set_t), &mask) == 0);
#endif

	while (!start) {}

	while (1) {
		assert(pthread_mutex_lock(&lock) == 0);
		while (buf_size != 0) {
			buf_size--;
			__asm__ volatile ("# read":::"memory");
			buf[buf_size];
			__asm__ volatile ("# read":::"memory");
			++num;
		}
		assert(pthread_mutex_unlock(&lock) == 0);
		if (num == SIZE)
			break;
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
		assert(pthread_mutex_lock(&lock) == 0);
		__asm__ volatile ("# write":::"memory");
		buf[buf_size] = i;
		__asm__ volatile ("# write":::"memory");
		buf_size++;
		assert(pthread_mutex_unlock(&lock) == 0);
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
	pthread_t tc[consumer_num];
	pthread_t tp[producer_num];

	printf("size %zu\n", SIZE);

	buf = malloc(sizeof(int) * SIZE);
	assert(buf);
	memset(buf, 1, sizeof(int) * SIZE);

	assert(pthread_mutex_init(&lock, NULL) == 0);

	for (size_t i = 0; i < (size_t)consumer_num; ++i)
		assert(pthread_create(&tc[i], NULL, consume, (void *)(uintptr_t)loop_time) == 0);

	for (size_t i = 0; i < (size_t)producer_num; ++i)
		assert(pthread_create(&tp[i], NULL, produce, (void *)(uintptr_t)loop_time) == 0);

#ifdef __linux__
	assert(mlockall(MCL_CURRENT | MCL_FUTURE) == 0);
#endif
	sleep(5);
	printf("start time: %llu\n", gettime_ns());
	start = 1;

	while (1)
		sleep(10000);

	return 0;
}
