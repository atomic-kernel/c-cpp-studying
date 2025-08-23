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

struct TT {
	volatile uintptr_t val0;
	uint64_t pad[4096];
	union test obj0;
	uint64_t pad1[4096];
	volatile uintptr_t val1;
} tt;
atomic_size_t count;

static void *thread0_0(void *arg)
{
	uintptr_t i = 0;
	struct raw raw;
	
retry:
	for (size_t t = 0; t < 10000; ++t) {
		++i;
		raw.low = i * 13241234 + 0xffee00000;
		raw.high = i;
		tt.val0 = i * 31415926 + 0xff0000000;
		tt.val1 = i * 32356256 + 0xee0000000;
#ifdef DEBUG
		__asm__ volatile ("":::"memory");
#endif
		atomic_store_explicit(&tt.obj0.atomic, raw, memory_order_release);
		while (1) {
			raw = atomic_load_explicit(&tt.obj0.atomic, memory_order_acquire);
#ifdef DEBUG
			__asm__ volatile ("":::"memory");
#endif
			if (raw.high != i) {
				assert(raw.high == ++i);
				assert(raw.low == (i - 1) * 13241234 + 0xffee00000);
				assert(tt.val0 == i * 31415926 + 0xff0000000);
				assert(tt.val1 == i * 32356256 + 0xee0000000);
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
		do {
			for (size_t i2 = 0; i2 < 20; ++i2) {
				__asm__ volatile (""::"r"(tt.val0):);
				__asm__ volatile (""::"r"(tt.val1):);
			}
			__asm__ volatile ("":::"memory");
			tmp2 = atomic_load_explicit(&tt.obj0.high, memory_order_acquire);
			__asm__ volatile ("":::"memory");
			for (size_t i2 = 0; i2 < 20; ++i2) {
				__asm__ volatile (""::"r"(tt.val0):);
				__asm__ volatile (""::"r"(tt.val1):);
			}
		} while (tmp2 == i);
#ifdef DEBUG
		__asm__ volatile ("":::"memory");
#endif
		assert(tmp2 == ++i);
		assert(tt.val0 == i * 31415926 + 0xff0000000);
		assert(tt.val1 == i * 32356256 + 0xee0000000);
		++i;
		tt.val0 = i * 31415926 + 0xff0000000;
		tt.val1 = i * 32356256 + 0xee0000000;
#ifdef DEBUG
		__asm__ volatile ("":::"memory");
#endif
		atomic_store_explicit(&tt.obj0.high, i, memory_order_release);
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
