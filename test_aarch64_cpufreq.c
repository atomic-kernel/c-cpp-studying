#include <stdio.h>
#include <time.h>
#include <assert.h>

static inline void asm_loop(uint64_t loop)
{
	__asm__ volatile (
			"1:\n\t"
			"subs %0, %0, #1\n\t"
			"b.ne	1b"
			:"+r"(loop)
			:
			:);
}

static inline unsigned long long gettime_ns(void)
{
	struct timespec ts;

	assert(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);

	return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
#define LOOP 50000000ULL

int main(void)
{
	unsigned long long min = -1ULL;
	for (size_t i = 0; i < 20; ++i) {
		const unsigned long long start_time = gettime_ns();
		asm_loop(LOOP);
		const unsigned long long cost = gettime_ns() - start_time;

		if (cost < min)
			min = cost;
	}
	printf("cost %llu ns, guest %Lf / %Lf ghz\n", min, (long double)LOOP / min, (long double)LOOP * 2 / min);

	return 0;
}
