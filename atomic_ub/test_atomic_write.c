#include <stdatomic.h>
#include <stddef.h>
#include <assert.h>
#include <stdint.h>

#include <pthread.h>

struct raw_obj_s
{
	uintptr_t low;
	uintptr_t high;
};

union obj_s {
	struct {
		atomic_uintptr_t low;
		atomic_uintptr_t high;
	};
	_Atomic struct raw_obj_s atomic;
};

union obj_s obj;

void *reader0(void *arg)
{
	while (1) {
		const uintptr_t low = atomic_load_explicit(&obj.low, memory_order_acquire);
		const uintptr_t high = atomic_load_explicit(&obj.high, memory_order_relaxed);
		assert(high - low < (uintptr_t)INTPTR_MAX);
	}
}

void *reader1(void *arg)
{
	while (1) {
		const uintptr_t high = atomic_load_explicit(&obj.high, memory_order_acquire);
		const uintptr_t low = atomic_load_explicit(&obj.low, memory_order_relaxed);
		assert(low - high < (uintptr_t)INTPTR_MAX);
	}
}

void writer(void)
{
	uintptr_t val = 1;
	while (1) {
		const struct raw_obj_s tmp = {val, val};
		atomic_store_explicit(&obj.atomic, tmp, memory_order_relaxed);
		++val;
	}
}

int main(void)
{
	pthread_t th[2];

	assert(pthread_create(&th[0], NULL, reader0, NULL) == 0);
	assert(pthread_create(&th[0], NULL, reader1, NULL) == 0);
	writer();
	return 0;
}
