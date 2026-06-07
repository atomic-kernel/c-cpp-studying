/*
 * Lock-free queue:
 * Allow multiple producers and consumers.
 * lqueue_init(): init a lock-free queue
 * lqueue_enqueue(): insert a node to the lock-free queue
 * lqueue_dequeue(): try dequeueing a node from the lock-free queue
 *
 * constraint:
 * 1. When a node is dequeued from a lock-free queue, it cannot be free,
 * unless it can be guaranteed that all operations of other threads
 * on this lock-free queue(including enqueue and dequeue) before this
 * moment(lqueue_dequeue() func returns) have been completed.
 *
 * If dynamically freeing the nodes is required, something similar to RCU
 * may be needed. It is recommended to reserve the memory of nodes, such as
 * defining them as static or global variables, or allocating the memory of
 * all nodes when initializing a lock-free queue,
 * and freeing all nodes when destroying the lock-free queue.
 *
 * 2. When a node is dequeued from a lock-free queue, the content of the node
 * itself(struct lqueue_node) cannot be modified unless the above freeing rule
 * is met. (An exception: dequeuing a node from a lock-free queue and then enqueue
 * it into another lock-free queue is allowed)
 */

#ifndef LQUEUE_H
#define LQUEUE_H

#include <stdatomic.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#if defined(__STDC_VERSION__) && __STDC_VERSION__ <= 201710L
#include <stdbool.h>
#endif

#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif

struct lqueue_node;

struct raw_lqueue_node {
	union {
		struct lqueue_node *next;
		uintptr_t unext;
	};
	uintptr_t count;
};

struct lqueue_node {
	union {
		struct {
			union {
				_Atomic(struct lqueue_node *) next;
				atomic_uintptr_t unext;
				struct lqueue_node *raw_next;
			};
			atomic_uintptr_t count;
		};
		_Atomic struct raw_lqueue_node node;
	};
};

struct lqueue {
	struct lqueue_node first;
	struct lqueue_node last;
};

_Static_assert(sizeof(struct lqueue_node) == 2 * sizeof(void *) &&
		alignof(struct lqueue_node) >= 2 * sizeof(void *) &&
		alignof(_Atomic struct raw_lqueue_node) == 2 * sizeof(void *),
		"size/align check failed");
#if UINTPTR_MAX == UINT64_MAX
# ifndef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
#  ifdef __x86_64__
#   warning "try to recompile with -mcx16 / -march=x86-64-v[2/3/4]"
#  else
#   warning "your platform may be not support 16b atomic"
#  endif
# endif
#elif !defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)
# if defined(__arm__)
#  warning "try to recompile with -march=armv7-a / -mcpu=cortex-a[7/9/15]"
# else
#  warning "your platform may be not support 8b atomic"
# endif
#endif
#if defined(__clang__) || UINTPTR_MAX == UINT32_MAX
_Static_assert(__atomic_always_lock_free(sizeof(_Atomic struct raw_lqueue_node),
			(void *)(uintptr_t)alignof(_Atomic struct raw_lqueue_node)),
		"lock free check failed");
#endif

static inline __attribute__((__always_inline__))
void lqueue_init_ex(struct lqueue *const q, void *const gnull)
{
	q->first.raw_next = gnull;
	atomic_init(&q->first.count, 0);
	q->last.raw_next = gnull;
	atomic_init(&q->last.count, 0);
}

// assume UINTPTR_MAX >= INTPTR_MAX
_Static_assert(UINTPTR_MAX == (uintptr_t)INTPTR_MAX ||
		(UINTPTR_MAX - (uintptr_t)INTPTR_MAX <= (uintptr_t)INTPTR_MAX + 1 &&
		 INTPTR_MIN + INTPTR_MAX < 0), "size check failed");
static inline __attribute__((__always_inline__)) intptr_t uptr_2_ptr(const uintptr_t x)
{
	if (x <= (uintptr_t)INTPTR_MAX)
		return x;
	return -(intptr_t)(UINTPTR_MAX - x) - 1;
}

// ptr should be pointer type or uintptr_t
#define PTR_ADD(ptr, off) ((__typeof__(ptr))((uintptr_t)(ptr) + (uintptr_t)(off)))
#define PTR_SUB(ptr, off) ((__typeof__(ptr))((uintptr_t)(ptr) - (uintptr_t)(off)))
#define VADDR_2_OFF(ptr) PTR_SUB(ptr, base_addr)
#define OFF_2_VADDR(ptr) PTR_ADD(ptr, base_addr)
// unext should be uintptr_t
#define LAST_REF(unext) OFF_2_VADDR((struct lqueue_node *)((uintptr_t)(unext) & (uintptr_t)-2))
#define NEED_PUSH_FIRST ((uintptr_t)(1UL << 0))
// x >= y
#define COUNT_GE(x, y) (uptr_2_ptr((uintptr_t)(x) - (uintptr_t)(y)) >= 0)
// x > y
#define COUNT_G(x, y) (uptr_2_ptr((uintptr_t)(x) - (uintptr_t)(y)) > 0)
// x < y
#define COUNT_S(x, y) (uptr_2_ptr((uintptr_t)(x) - (uintptr_t)(y)) < 0)
// x <= y
#define COUNT_SE(x, y) (uptr_2_ptr((uintptr_t)(x) - (uintptr_t)(y)) <= 0)

static inline __attribute__((__always_inline__))
bool push_first(struct lqueue *const q, const uintptr_t min, struct raw_lqueue_node last, const memory_order read_order, const memory_order write_order, const bool expect_already_done)
{
	struct raw_lqueue_node old_first;

	old_first.count = atomic_load_explicit(&q->first.count, read_order);
	if (__builtin_expect(COUNT_GE(old_first.count, min), !!expect_already_done))
		return COUNT_G(old_first.count, last.count);
	old_first.next = q->first.raw_next;

	last.unext &= (uintptr_t)-2;

	do {
		if (likely(atomic_compare_exchange_weak_explicit(&q->first.node, &old_first, last, write_order, read_order)))
			return false;
	} while (COUNT_S(old_first.count, min));

	return COUNT_G(old_first.count, last.count);
}

static inline __attribute__((__always_inline__))
bool lqueue_enqueue_ex(struct lqueue *const q, struct lqueue_node *const new_node, void *const qnull, void *const gnull, void *const base_addr)
{
	struct raw_lqueue_node old_last;
	struct raw_lqueue_node new_last;
	struct raw_lqueue_node old_next;
	struct raw_lqueue_node new_next;

	if ((uintptr_t)base_addr & 0xfff)
		__builtin_unreachable();

restart:
	old_last.count = atomic_load_explicit(&q->last.count, memory_order_acquire);
	old_last.unext = atomic_load_explicit(&q->last.unext, memory_order_relaxed);
retry:
	if (old_last.unext == (uintptr_t)gnull) {
		atomic_store_explicit(&new_node->count, old_last.count + 1, memory_order_relaxed);
		atomic_store_explicit(&new_node->next, qnull, memory_order_release);
		new_last.unext = (uintptr_t)VADDR_2_OFF(new_node) | NEED_PUSH_FIRST;
		new_last.count = old_last.count + 1;
		if (likely(atomic_compare_exchange_weak_explicit(&q->last.node, &old_last, new_last, memory_order_release, memory_order_acquire)))
			return true;
		goto retry;
	}

	if (old_last.unext & NEED_PUSH_FIRST) {
		old_next.next = atomic_load_explicit(&LAST_REF(old_last.unext)->next, memory_order_acquire);
		if (unlikely(old_next.next != qnull))
			goto failed;

		// push first
		if (unlikely(push_first(q, old_last.count, old_last, memory_order_acquire, memory_order_release, true)))
			goto restart;
	}
	atomic_store_explicit(&new_node->count, old_last.count + 1, memory_order_relaxed);
	atomic_store_explicit(&new_node->next, qnull, memory_order_release);
	new_next.next = VADDR_2_OFF(new_node);
	new_next.count = old_last.count;
	old_next.next = qnull;
	old_next.count = old_last.count;
	if (unlikely(!atomic_compare_exchange_strong_explicit(&LAST_REF(old_last.unext)->node, &old_next, new_next, memory_order_release, memory_order_acquire)))
		goto failed;

	new_last.next = VADDR_2_OFF(new_node);
	new_last.count = old_last.count + 1;
	atomic_compare_exchange_weak_explicit(&q->last.node, &old_last, new_last, memory_order_release, memory_order_relaxed);
	return false;

failed:
	if (unlikely(old_next.next == gnull)) {
		atomic_store_explicit(&new_node->count, old_last.count + 2, memory_order_relaxed);
		atomic_store_explicit(&new_node->next, qnull, memory_order_release);
		new_last.unext = (uintptr_t)VADDR_2_OFF(new_node) | 1;
		new_last.count = old_last.count + 2;
		if (atomic_compare_exchange_weak_explicit(&q->last.node, &old_last, new_last, memory_order_release, memory_order_acquire))
			return true;
	} else {
		new_last.next = old_next.next;
		new_last.count = old_last.count + 1;
		if (atomic_compare_exchange_weak_explicit(&q->last.node, &old_last, new_last, memory_order_release, memory_order_acquire)) {
			assert(new_last.next != qnull);
			assert(new_last.next != gnull);
			old_last = new_last;
		}
	}
	goto retry;
}

struct lqueue_dequeue_ret {
	struct lqueue_node *node;
	bool is_last;
};

static inline __attribute__((__always_inline__))
struct lqueue_dequeue_ret lqueue_dequeue_ex(struct lqueue *const q, void *const qnull, void *const gnull, void *const base_addr)
{
	struct raw_lqueue_node old_first;
	struct raw_lqueue_node new_first;
	struct raw_lqueue_node old_last;
	struct raw_lqueue_node new_last;
	struct raw_lqueue_node old_last_next;
	struct raw_lqueue_node new_last_next;
	struct lqueue_node *first_next;

	if ((uintptr_t)base_addr & 0xfff)
		__builtin_unreachable();

restart:
	old_first.next = atomic_load_explicit(&q->first.next, memory_order_relaxed);
	assert(old_first.unext % alignof(struct lqueue_node) == 0);
	old_first.count = atomic_load_explicit(&q->first.count, memory_order_relaxed);
retry_read_last:
	old_last.count = atomic_load_explicit(&q->last.count, memory_order_acquire);
	old_last.unext = atomic_load_explicit(&q->last.unext, memory_order_relaxed);
retry_last_got:
	if (old_last.unext == (uintptr_t)gnull) {
		struct lqueue_dequeue_ret ret;

		ret.node = NULL;
		return ret;
	}
	if ((old_last.unext & NEED_PUSH_FIRST) || COUNT_GE(old_first.count, old_last.count))
		goto try_last;

retry_dequeue_first:
	if (unlikely(old_first.next == gnull))
		goto restart;

	first_next = old_first.next->raw_next;
	new_first.next = first_next;
	new_first.count = old_first.count + 1;
	if (likely(atomic_compare_exchange_weak_explicit(&q->first.node, &old_first, new_first, memory_order_relaxed, memory_order_relaxed))) {
		assert(new_first.unext % alignof(struct lqueue_node) == 0);
		assert(new_first.next != qnull);
		assert(new_first.next != gnull);
		return (struct lqueue_dequeue_ret){old_first.next, false};
	}

	if (COUNT_GE(old_first.count, old_last.count))
		goto retry_read_last;
	goto retry_dequeue_first;

try_last:
	old_last_next.next = qnull;
	old_last_next.count = old_last.count;
	new_last_next.next = gnull;
	new_last_next.count = old_last.count;
	if (likely(atomic_compare_exchange_strong_explicit(&LAST_REF(old_last.unext)->node, &old_last_next, new_last_next, memory_order_acquire, memory_order_acquire))) {
		const struct lqueue_dequeue_ret ret = {LAST_REF(old_last.unext), true};
		const uintptr_t min = old_last.count + 1;

		new_last.next = gnull;
		new_last.count = old_last.count + 1;
		// order: success should >= failed
		if (unlikely(!atomic_compare_exchange_strong_explicit(&q->last.node, &old_last, new_last, memory_order_acquire, memory_order_acquire))) {
			assert(COUNT_GE(old_last.count, new_last.count));
			if (!(old_last.unext & NEED_PUSH_FIRST) && old_last.next != gnull) {
				assert(COUNT_GE(atomic_load_explicit(&q->first.count, memory_order_relaxed), new_last.count));
				goto skip;
			}
			new_last = old_last;
		}
		push_first(q, min, new_last, memory_order_relaxed, memory_order_relaxed, false);
skip:
		return ret;
	}
	if (old_last_next.next == gnull) {
		new_last.next = gnull;
		new_last.count = old_last.count + 1;
		if (atomic_compare_exchange_weak_explicit(&q->last.node, &old_last, new_last, memory_order_release, memory_order_acquire)) {
			struct lqueue_dequeue_ret ret;

			ret.node = NULL;
			return ret;
		}
	} else {
		new_last.next = old_last_next.next;
		new_last.count = old_last.count + 1;
		if (atomic_compare_exchange_weak_explicit(&q->last.node, &old_last, new_last, memory_order_release, memory_order_acquire)) {
			assert(new_last.next != qnull);
			assert(new_last.next != gnull);
			old_last = new_last;
		}
	}
	goto retry_last_got;
}

#undef PTR_ADD
#undef PTR_SUB
#undef VADDR_2_OFF
#undef OFF_2_VADDR
#undef LAST_REF

static inline __attribute__((__always_inline__))
void lqueue_init(struct lqueue *const q)
{
	lqueue_init_ex(q, NULL);
}

static inline __attribute__((__always_inline__))
bool lqueue_enqueue(struct lqueue *const q, struct lqueue_node *const new_node)
{
	return lqueue_enqueue_ex(q, new_node, q, NULL, (void *)(uintptr_t)0);
}

static inline __attribute__((__always_inline__))
struct lqueue_dequeue_ret lqueue_dequeue(struct lqueue *const q)
{
	return lqueue_dequeue_ex(q, q, NULL, (void *)(uintptr_t)0);
}
#endif
