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
#if defined(__STDC_VERSION__) && __STDC_VERSION__ <= 201710L
#include <stdbool.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdalign.h>

#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif

#ifndef LQUEUE_NDEBUG
#define LQUEUE_DEBUG 1
#endif

#define WRITE(fd, str) write(fd, str, sizeof(str) - 1)
#define XSTR(s) #s
#define STR(s) XSTR(s)

#ifdef LQUEUE_DEBUG
#define LQUEUE_ASSERT(exp) \
	do { \
		if (__builtin_expect_with_probability(!(exp), 0, 1)) { \
			WRITE(2, __FILE__ ":" STR(__LINE__) ": '" #exp "' failed\n"); \
			abort(); \
		} \
	} while (0)
#else
#define LQUEUE_ASSERT(exp) do {} while (0)
#endif

struct raw_lqueue_node {
	uintptr_t next;
	uintptr_t count;
};

struct lqueue_node {
	union {
		struct {
			union {
				atomic_uintptr_t next;
				uintptr_t raw_next;
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

static_assert(sizeof(struct lqueue_node) == 2 * sizeof(void *) &&
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
static_assert(__atomic_always_lock_free(sizeof(_Atomic struct raw_lqueue_node),
			(void *)(uintptr_t)alignof(_Atomic struct raw_lqueue_node)),
		"lock free check failed");
#endif

typedef struct lqueue_node *(*element_to_node_func_t)(void *element);

static inline __attribute__((__always_inline__))
struct lqueue_node *default_element_to_node(void *const element)
{
	return (struct lqueue_node *)element;
}

static inline __attribute__((__always_inline__))
void lqueue_init_ex(struct lqueue *const q, void *const gnull)
{
	LQUEUE_ASSERT((uintptr_t)gnull % alignof(_Atomic struct raw_lqueue_node) == 0);
	q->first.raw_next = (uintptr_t)gnull;
	atomic_init(&q->first.count, 0);
	q->last.raw_next = (uintptr_t)gnull;
	atomic_init(&q->last.count, 0);
}

// assume UINTPTR_MAX >= INTPTR_MAX
static_assert(UINTPTR_MAX == (uintptr_t)INTPTR_MAX ||
		(UINTPTR_MAX - (uintptr_t)INTPTR_MAX <= (uintptr_t)INTPTR_MAX + 1 &&
		 INTPTR_MIN + INTPTR_MAX < 0), "size check failed");
static inline __attribute__((__always_inline__)) intptr_t uptr_2_ptr(const uintptr_t x)
{
	if (x <= (uintptr_t)INTPTR_MAX)
		return x;
	return -(intptr_t)(UINTPTR_MAX - x) - 1;
}

// ptr should be void * (element)
#define VADDR_2_OFF(ptr) ((uintptr_t)((uintptr_t)(ptr) - (uintptr_t)base_addr))
// off should be uintptr_t (next)
#define OFF_2_VADDR(off) ((void *)((uintptr_t)(off) + (uintptr_t)base_addr))
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
bool push_first_enqueue(struct lqueue *const q, struct raw_lqueue_node last)
{
	struct raw_lqueue_node old_head_first;

	old_head_first.count = atomic_load_explicit(&q->first.count, memory_order_acquire);
	if (COUNT_GE(old_head_first.count, last.count))
		return unlikely(COUNT_G(old_head_first.count, last.count));
	old_head_first.next = q->first.raw_next;

	last.next &= (uintptr_t)-2;

	do {
		if (likely(atomic_compare_exchange_weak_explicit(&q->first.node, &old_head_first, last, memory_order_release, memory_order_acquire)))
			return false;
	} while (COUNT_S(old_head_first.count, last.count));

	return COUNT_G(old_head_first.count, last.count);
}

static inline __attribute__((__always_inline__))
void push_first_dequeue(struct lqueue *const q, const uintptr_t min,
		struct raw_lqueue_node *const last, struct raw_lqueue_node *const old_head_first)
{
	if (unlikely(COUNT_GE(old_head_first->count, min))) /* See: https://stackoverflow.com/questions/79958831 */
		return;

	last->next &= (uintptr_t)-2;

	do {
		if (likely(atomic_compare_exchange_weak_explicit(&q->first.node, old_head_first, *last, memory_order_relaxed, memory_order_relaxed)))
			return;
	} while (unlikely(COUNT_S(old_head_first->count, min)));
}

static inline __attribute__((__always_inline__))
bool lqueue_enqueue_ex(struct lqueue *const q, void *const new_element,
		void *const qnull, void *const gnull, void *const base_addr,
		const element_to_node_func_t element_to_node)
{
	struct raw_lqueue_node old_head_last;
	struct raw_lqueue_node new_head_last;
	struct raw_lqueue_node old_last_node;
	struct raw_lqueue_node new_last_node;
	struct lqueue_node *const new_node = element_to_node(new_element);
	const uintptr_t new_pnext = VADDR_2_OFF(new_element);
#ifdef LQUEUE_DEBUG
	struct raw_lqueue_node old_head_last_bak;
#endif

	LQUEUE_ASSERT(((uintptr_t)base_addr & 0xfff) == 0); /* base_addr should aligned to page_size */
	LQUEUE_ASSERT((uintptr_t)new_element % alignof(_Atomic struct raw_lqueue_node) == 0);
	static_assert(alignof(_Atomic struct raw_lqueue_node) <= 0x1000, "align check error");
	/* new_pnext % alignof(_Atomic struct raw_lqueue_node) should == 0 */
	LQUEUE_ASSERT(qnull != gnull && (uintptr_t)qnull % alignof(_Atomic struct raw_lqueue_node) == 0);
	LQUEUE_ASSERT(new_pnext != (uintptr_t)qnull && new_pnext != (uintptr_t)gnull);
	/* (new_pnext | NEED_PUSH_FIRST) should also != qnull/gnull */

restart:
	old_head_last.count = atomic_load_explicit(&q->last.count, memory_order_acquire);
	old_head_last.next = atomic_load_explicit(&q->last.next, memory_order_relaxed);
retry:
#ifdef LQUEUE_DEBUG
	old_head_last_bak = old_head_last;
#endif
	if (old_head_last.next == (uintptr_t)gnull) {
		atomic_store_explicit(&new_node->count, old_head_last.count + 1, memory_order_relaxed);
		atomic_store_explicit(&new_node->next, (uintptr_t)qnull, memory_order_release);
		new_head_last.next = new_pnext | NEED_PUSH_FIRST;
		new_head_last.count = old_head_last.count + 1;
		if (likely(atomic_compare_exchange_weak_explicit(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_acquire)))
			return true;
		LQUEUE_ASSERT(COUNT_GE(old_head_last.count, old_head_last_bak.count));
		LQUEUE_ASSERT(old_head_last.count != old_head_last_bak.count || old_head_last.next == old_head_last_bak.next);
		goto retry;
	}

	struct lqueue_node *const last_node = element_to_node(OFF_2_VADDR(old_head_last.next & (uintptr_t)-2));
	if (old_head_last.next & NEED_PUSH_FIRST) {
		old_last_node.next = atomic_load_explicit(&last_node->next, memory_order_acquire);
		if (unlikely(old_last_node.next != (uintptr_t)qnull))
			goto failed;

		if (push_first_enqueue(q, old_head_last))
			goto restart;
	}
	atomic_store_explicit(&new_node->count, old_head_last.count + 1, memory_order_relaxed);
	atomic_store_explicit(&new_node->next, (uintptr_t)qnull, memory_order_release);
	new_last_node.next = new_pnext;
	new_last_node.count = old_head_last.count;
	old_last_node.next = (uintptr_t)qnull;
	old_last_node.count = old_head_last.count;
	if (unlikely(!atomic_compare_exchange_strong_explicit(&last_node->node, &old_last_node, new_last_node, memory_order_release, memory_order_acquire)))
		goto failed;

	new_head_last.next = new_pnext;
	new_head_last.count = old_head_last.count + 1;
	if (unlikely(!atomic_compare_exchange_weak_explicit(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_relaxed))) {
		LQUEUE_ASSERT(COUNT_GE(old_head_last.count, old_head_last_bak.count));
		LQUEUE_ASSERT(old_head_last.count != old_head_last_bak.count || old_head_last.next == old_head_last_bak.next);
		LQUEUE_ASSERT(old_head_last.count != new_head_last.count || old_head_last.next == new_pnext);
	}
	return false;

failed:
	if (unlikely(old_last_node.next == (uintptr_t)gnull)) {
		atomic_store_explicit(&new_node->count, old_head_last.count + 2, memory_order_relaxed);
		atomic_store_explicit(&new_node->next, (uintptr_t)qnull, memory_order_release);
		new_head_last.next = new_pnext | NEED_PUSH_FIRST;
		new_head_last.count = old_head_last.count + 2;
		if (atomic_compare_exchange_weak_explicit(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_acquire))
			return true;
		LQUEUE_ASSERT(COUNT_GE(old_head_last.count, old_head_last_bak.count));
		LQUEUE_ASSERT(old_head_last.count != old_head_last_bak.count || old_head_last.next == old_head_last_bak.next);
	} else {
		new_head_last.next = old_last_node.next;
		new_head_last.count = old_head_last.count + 1;
		if (atomic_compare_exchange_weak_explicit(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_acquire)) {
			LQUEUE_ASSERT(new_head_last.next != (uintptr_t)qnull);
			LQUEUE_ASSERT(new_head_last.next % alignof(_Atomic struct raw_lqueue_node) == 0);
			old_head_last = new_head_last;
		} else {
			LQUEUE_ASSERT(COUNT_GE(old_head_last.count, old_head_last_bak.count));
			LQUEUE_ASSERT(old_head_last.count != old_head_last_bak.count || old_head_last.next == old_head_last_bak.next);
		}
	}
	goto retry;
}

struct lqueue_dequeue_ex_ret {
	void *element;
	bool is_last;
};

static inline __attribute__((__always_inline__))
struct lqueue_dequeue_ex_ret lqueue_dequeue_ex(struct lqueue *const q,
		void *const qnull, void *const gnull, void *const base_addr,
		element_to_node_func_t element_to_node)
{
	struct raw_lqueue_node old_head_first;
	struct raw_lqueue_node new_head_first;
	struct raw_lqueue_node old_head_last;
	struct raw_lqueue_node new_head_last;
	struct raw_lqueue_node old_last_node;
	struct raw_lqueue_node new_last_node;
	uintptr_t first_pnext;

	LQUEUE_ASSERT(((uintptr_t)base_addr & 0xfff) == 0);

restart:
	old_head_first.next = atomic_load_explicit(&q->first.next, memory_order_relaxed);
	old_head_first.count = atomic_load_explicit(&q->first.count, memory_order_relaxed);
retry_read_last:
	old_head_last.count = atomic_load_explicit(&q->last.count, memory_order_acquire);
	old_head_last.next = atomic_load_explicit(&q->last.next, memory_order_relaxed);
retry_last_got:
	if (old_head_last.next == (uintptr_t)gnull) {
		struct lqueue_dequeue_ex_ret ret;

		ret.element = NULL;
		return ret;
	}
	if ((old_head_last.next & NEED_PUSH_FIRST) || COUNT_GE(old_head_first.count, old_head_last.count))
		goto try_last;

retry_dequeue_first:
	if (unlikely(old_head_first.next == (uintptr_t)gnull))
		goto restart;

	first_pnext = element_to_node((void *)OFF_2_VADDR(old_head_first.next))->raw_next;
	new_head_first.next = first_pnext;
	new_head_first.count = old_head_first.count + 1;
	if (likely(atomic_compare_exchange_weak_explicit(&q->first.node, &old_head_first, new_head_first, memory_order_relaxed, memory_order_relaxed))) {
		LQUEUE_ASSERT(new_head_first.next != (uintptr_t)qnull);
		LQUEUE_ASSERT(new_head_first.next != (uintptr_t)gnull);
		LQUEUE_ASSERT(new_head_first.next % alignof(_Atomic struct raw_lqueue_node) == 0);
		return (struct lqueue_dequeue_ex_ret){OFF_2_VADDR(old_head_first.next), false};
	}

	if (COUNT_GE(old_head_first.count, old_head_last.count))
		goto retry_read_last;
	goto retry_dequeue_first;

try_last:
	old_last_node.next = (uintptr_t)qnull;
	old_last_node.count = old_head_last.count;
	new_last_node.next = (uintptr_t)gnull;
	new_last_node.count = old_head_last.count;
	if (likely(atomic_compare_exchange_strong_explicit(&element_to_node(OFF_2_VADDR(old_head_last.next & -2))->node, &old_last_node, new_last_node, memory_order_acquire, memory_order_acquire))) {
		const struct lqueue_dequeue_ex_ret ret = {OFF_2_VADDR(old_head_last.next & -2), true};
		const uintptr_t min = old_head_last.count + 1;

		new_head_last.next = (uintptr_t)gnull;
		new_head_last.count = old_head_last.count + 1;
		// order: success should >= failed
		if (unlikely(!atomic_compare_exchange_strong_explicit(&q->last.node, &old_head_last, new_head_last, memory_order_acquire, memory_order_acquire))) {
			LQUEUE_ASSERT(COUNT_GE(old_head_last.count, new_head_last.count));
			LQUEUE_ASSERT(old_head_last.count != new_head_last.count || old_head_last.next == (uintptr_t)gnull);
			if (!(old_head_last.next & NEED_PUSH_FIRST) && old_head_last.next != (uintptr_t)gnull) {
				LQUEUE_ASSERT(COUNT_GE(atomic_load_explicit(&q->first.count, memory_order_relaxed), min));
				goto skip;
			}
			new_head_last = old_head_last;
		}
		push_first_dequeue(q, min, &new_head_last, &old_head_first);
skip:
		return ret;
	}
	if (old_last_node.next == (uintptr_t)gnull) {
		new_head_last.next = (uintptr_t)gnull;
		new_head_last.count = old_head_last.count + 1;
		if (atomic_compare_exchange_weak_explicit(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_acquire)) {
			struct lqueue_dequeue_ex_ret ret;

			ret.element = NULL;
			return ret;
		}
	} else {
		new_head_last.next = old_last_node.next;
		new_head_last.count = old_head_last.count + 1;
		if (atomic_compare_exchange_weak_explicit(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_acquire)) {
			LQUEUE_ASSERT(new_head_last.next != (uintptr_t)qnull);
			/* omit: new_head_last.next should also != qnull */
			old_head_last = new_head_last;
		}
	}
	goto retry_last_got;
}

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
	return lqueue_enqueue_ex(q, new_node, q, NULL, (void *)(uintptr_t)0, default_element_to_node);
}

struct lqueue_dequeue_ret {
	struct lqueue_node *node;
	bool is_last;
};
static inline __attribute__((__always_inline__))
struct lqueue_dequeue_ret lqueue_dequeue(struct lqueue *const q)
{
	const struct lqueue_dequeue_ex_ret ret = lqueue_dequeue_ex(q, q, NULL, (void *)(uintptr_t)0, default_element_to_node);

	return (struct lqueue_dequeue_ret){ret.element, ret.is_last};
}
#endif
