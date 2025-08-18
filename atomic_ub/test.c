#include <stdatomic.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <pthread.h>

struct raw {
	uintptr_t low;
	uintptr_t high;
};
union test {
	struct {
		atomic_uintptr_t low;
		atomic_uintptr_t high;
	};
	_Atomic struct raw atomic;
};

uintptr_t val = 5;
union test obj0 = { {0, 0} };
atomic_size_t count;

static void *thread0_0(void *arg)
{
	uintptr_t i = 0;
	struct raw raw;
	
retry:
	for (size_t t = 0; t < 10000; ++t) {
		++i;
		raw.low = i + 7;
		raw.high = i;
		++val;
		atomic_store_explicit(&obj0.atomic, raw, memory_order_release);
		while (1) {
			raw = atomic_load_explicit(&obj0.atomic, memory_order_acquire);
			if (raw.high != i) {
				assert(val == raw.high + 5);
				assert(raw.high == ++i);
				assert(raw.low == i + 6);
				break;
			}
		}
	}
	atomic_fetch_add_explicit(&count, 1, memory_order_relaxed);
	goto retry;
	__builtin_unreachable();
}
static void *thread0_1(void *arg)
{
	uintptr_t i = 0;
	uintptr_t tmp2;
	while (1) {
		while ((tmp2 = atomic_load_explicit(&obj0.high, memory_order_acquire)) == i)
			;
		assert(val == tmp2 + 5);
		assert(tmp2 == ++i);
		++val;
		++i;
		atomic_store_explicit(&obj0.high, i, memory_order_release);
	}
	__builtin_unreachable();
}

int main(void)
{
	pthread_t th[2];
	size_t old_count, tmp;

	assert(pthread_create(&th[0], NULL, thread0_0, NULL) == 0);
	assert(pthread_create(&th[1], NULL, thread0_1, NULL) == 0);

	old_count = atomic_load_explicit(&count, memory_order_relaxed);
	while (1) {
		sleep(10);
		tmp = atomic_load_explicit(&count, memory_order_relaxed);
		assert(tmp != old_count);
		printf("count : %zu\n", tmp);
		old_count = tmp;
	}
	return 0;
}
