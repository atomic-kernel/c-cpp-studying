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
#ifndef unlikely_ex
#define unlikely_ex(x, p)	__builtin_expect_with_probability(!!(x), 0, p)
#endif

#define CACHELINE_SIZE 64

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
#define lqueue_cmpxchg_weak(a, b, c, d, e) atomic_compare_exchange_strong_explicit((a), (b), (c), (d), (e))
#else
#define LQUEUE_ASSERT(exp) do {} while (0)
#define lqueue_cmpxchg_weak(a, b, c, d, e) atomic_compare_exchange_weak_explicit((a), (b), (c), (d), (e))
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
	struct lqueue_node first __attribute__((aligned(CACHELINE_SIZE)));
	struct lqueue_node last __attribute__((aligned(CACHELINE_SIZE)));
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
	LQUEUE_ASSERT((uintptr_t)gnull % alignof(struct lqueue_node) == 0);
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
		/*
		 * write release: make sure that:
		 *    r1 = load_acquire(head_first.count);
		 *    r2 = load_relaxed(head_last.count);
		 *    assert(r2 >= r1);
		 *
		 * read relaxed: OoTA is forbidden: https://stackoverflow.com/questions/79968377
		 */
		if (likely(atomic_compare_exchange_weak_explicit(&q->first.node, &old_head_first, last, memory_order_release, memory_order_relaxed)))
			return false;
	} while (COUNT_S(old_head_first.count, last.count));

	return COUNT_G(old_head_first.count, last.count);
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
	struct lqueue_node *last_node;

	LQUEUE_ASSERT(((uintptr_t)base_addr & 0xfff) == 0); /* base_addr should aligned to page_size */
	LQUEUE_ASSERT((uintptr_t)new_element % alignof(struct lqueue_node) == 0);
	static_assert(alignof(struct lqueue_node) <= 0x1000, "align check error");
	/* new_pnext % alignof(struct lqueue_node) should == 0 */
	LQUEUE_ASSERT(qnull != gnull && (uintptr_t)qnull != (uintptr_t)-1);
	LQUEUE_ASSERT(new_pnext != (uintptr_t)qnull && new_pnext != (uintptr_t)gnull);

	// TODO: add init doc
	LQUEUE_ASSERT(atomic_load_explicit(&new_node->next, memory_order_relaxed) == (uintptr_t)-1);

restart:
	old_head_last.count = atomic_load_explicit(&q->last.count, memory_order_acquire);
	old_head_last.next = atomic_load_explicit(&q->last.next, memory_order_relaxed);
retry:
#ifdef LQUEUE_DEBUG
	last_node = element_to_node(OFF_2_VADDR(old_head_last.next & (uintptr_t)-2));
	LQUEUE_ASSERT(last_node != new_node);
#endif
	atomic_store_explicit(&new_node->count, old_head_last.count + 1, memory_order_relaxed);
	atomic_store_explicit(&new_node->next, (uintptr_t)qnull, memory_order_release);
	if (old_head_last.next == (uintptr_t)gnull) {
		new_head_last.next = new_pnext | NEED_PUSH_FIRST;
		new_head_last.count = old_head_last.count + 1;
		if (likely(lqueue_cmpxchg_weak(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_acquire)))
			return true;
		LQUEUE_ASSERT(COUNT_GE(old_head_last.count, new_head_last.count));
		goto retry;
	}

#ifndef LQUEUE_DEBUG
	last_node = element_to_node(OFF_2_VADDR(old_head_last.next & (uintptr_t)-2));
#endif
	if (old_head_last.next & NEED_PUSH_FIRST) {
		old_last_node.next = atomic_load_explicit(&last_node->next, memory_order_acquire);
		if (unlikely(old_last_node.next != (uintptr_t)qnull))
			goto failed;

		if (push_first_enqueue(q, old_head_last))
			goto restart;
	}
	new_last_node.next = new_pnext;
	new_last_node.count = old_head_last.count;
	old_last_node.next = (uintptr_t)qnull;
	old_last_node.count = old_head_last.count;
	if (unlikely(!atomic_compare_exchange_strong_explicit(&last_node->node, &old_last_node, new_last_node, memory_order_release, memory_order_acquire)))
		goto failed;

	new_head_last.next = new_pnext;
	new_head_last.count = old_head_last.count + 1;
	if (unlikely(!lqueue_cmpxchg_weak(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_relaxed))) {
		LQUEUE_ASSERT(COUNT_GE(old_head_last.count, new_head_last.count));
		LQUEUE_ASSERT(old_head_last.count != new_head_last.count || old_head_last.next == new_pnext);
	}
	return false;

failed:
	if (unlikely(old_last_node.next == (uintptr_t)gnull)) {
		atomic_store_explicit(&new_node->count, old_head_last.count + 2, memory_order_relaxed);
		new_head_last.next = new_pnext | NEED_PUSH_FIRST;
		new_head_last.count = old_head_last.count + 2;
		if (lqueue_cmpxchg_weak(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_acquire))
			return true;
		LQUEUE_ASSERT(COUNT_GE(old_head_last.count, new_head_last.count - 1));
	} else {
		new_head_last.next = old_last_node.next;
		new_head_last.count = old_head_last.count + 1;
		if (lqueue_cmpxchg_weak(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_acquire)) {
			LQUEUE_ASSERT(!(new_head_last.next & (alignof(struct lqueue_node) - 1)));
			LQUEUE_ASSERT(new_head_last.next != (uintptr_t)qnull);
			old_head_last = new_head_last;
		}
		LQUEUE_ASSERT(COUNT_GE(old_head_last.count, new_head_last.count));
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
		const element_to_node_func_t element_to_node)
{
	struct raw_lqueue_node old_head_first;
	struct raw_lqueue_node new_head_first;
	struct raw_lqueue_node old_head_last;
	struct raw_lqueue_node new_head_last;
	struct raw_lqueue_node old_last_node;
	struct raw_lqueue_node new_last_node;
	uintptr_t first_pnext;
	uintptr_t cached_nr_elements;

	LQUEUE_ASSERT(((uintptr_t)base_addr & 0xfff) == 0);

restart:
	old_head_first.next = atomic_load_explicit(&q->first.next, memory_order_relaxed);
	old_head_first.count = atomic_load_explicit(&q->first.count, memory_order_acquire);
	cached_nr_elements = old_head_first.next & (uintptr_t)(alignof(struct lqueue_node) - 1);
	if (cached_nr_elements) {
		old_head_last.count = old_head_first.count;
		goto fast_path;
	}

retry_read_last:
	old_head_last.count = atomic_load_explicit(&q->last.count, memory_order_acquire);
#ifndef LQUEUE_NEED_FREE
	LQUEUE_ASSERT(COUNT_SE(old_head_first.count, old_head_last.count));
#endif
	old_head_last.next = atomic_load_explicit(&q->last.next, memory_order_acquire);
retry_last_got:
	if (old_head_last.next == (uintptr_t)gnull) {
		struct lqueue_dequeue_ex_ret ret;

		ret.element = NULL;
		return ret;
	}
	if ((old_head_last.next & NEED_PUSH_FIRST) || old_head_first.count == old_head_last.count)
		goto try_last;

	if (unlikely_ex(old_head_first.next == (uintptr_t)gnull, 1))
		goto restart;

retry_dequeue_first:
	cached_nr_elements = old_head_last.count - old_head_first.count;
	if (cached_nr_elements > alignof(struct lqueue_node))
		cached_nr_elements = alignof(struct lqueue_node);

fast_path: // cache hit
	--cached_nr_elements;
	first_pnext = element_to_node((void *)OFF_2_VADDR(old_head_first.next & (uintptr_t)-alignof(struct lqueue_node)))->raw_next;
	new_head_first.next = first_pnext | cached_nr_elements;
	new_head_first.count = old_head_first.count + 1;
	if (likely(atomic_compare_exchange_weak_explicit(&q->first.node, &old_head_first, new_head_first, memory_order_release, memory_order_acquire))) {
		LQUEUE_ASSERT(!(first_pnext & (alignof(struct lqueue_node) - 1)));
		LQUEUE_ASSERT(first_pnext != (uintptr_t)qnull);
		LQUEUE_ASSERT(first_pnext != (uintptr_t)gnull);
#ifdef LQUEUE_DEBUG
		atomic_store_explicit(&element_to_node(OFF_2_VADDR(old_head_first.next & (uintptr_t)-alignof(struct lqueue_node)))->next, -1, memory_order_relaxed);
#endif
		return (struct lqueue_dequeue_ex_ret){OFF_2_VADDR(old_head_first.next & (uintptr_t)-alignof(struct lqueue_node)), false};
	}
	LQUEUE_ASSERT(COUNT_GE(old_head_first.count, new_head_first.count - 1));

	cached_nr_elements = old_head_first.next & (uintptr_t)(alignof(struct lqueue_node) - 1);
	if (cached_nr_elements)
		goto fast_path;
	if (COUNT_S(old_head_first.count, old_head_last.count)) {
		LQUEUE_ASSERT(old_head_first.next != (uintptr_t)gnull);
		goto retry_dequeue_first;
	}
	goto retry_read_last;

try_last:
	old_last_node.next = (uintptr_t)qnull;
	old_last_node.count = old_head_last.count;
	new_last_node.next = (uintptr_t)gnull;
	new_last_node.count = old_head_last.count;
	void *const last_element = OFF_2_VADDR(old_head_last.next & (uintptr_t)-2);
	if (likely(atomic_compare_exchange_strong_explicit(&element_to_node(last_element)->node, &old_last_node, new_last_node, memory_order_acquire, memory_order_acquire))) {
		new_head_last.next = (uintptr_t)gnull;
		new_head_last.count = old_head_last.count + 1;
		// read acquire: or relaxed + if and OoTA is forbidden
		// write acquire: write relaxed is ok, but cmpxchg need success >= failed
		if (unlikely(!atomic_compare_exchange_strong_explicit(&q->last.node, &old_head_last, new_head_last, memory_order_acquire, memory_order_acquire))) {
			LQUEUE_ASSERT(COUNT_GE(old_head_last.count, new_head_last.count));
			LQUEUE_ASSERT(old_head_last.count != new_head_last.count || old_head_last.next == (uintptr_t)gnull);
		}
#ifdef LQUEUE_DEBUG
		atomic_store_explicit(&element_to_node(last_element)->next, -1, memory_order_relaxed);
#endif
		return (struct lqueue_dequeue_ex_ret){last_element, true};
	}
	new_head_last.next = old_last_node.next;
	new_head_last.count = old_head_last.count + 1;
	if (lqueue_cmpxchg_weak(&q->last.node, &old_head_last, new_head_last, memory_order_release, memory_order_acquire)) {
		LQUEUE_ASSERT(old_last_node.count == old_head_last.count);
		if (new_head_last.next == (uintptr_t)gnull) {
			struct lqueue_dequeue_ex_ret ret;

			ret.element = NULL;
			return ret;
		}
		LQUEUE_ASSERT(!(new_head_last.next & (alignof(struct lqueue_node) - 1)));
		LQUEUE_ASSERT(new_head_last.next != (uintptr_t)qnull);
		/* omit: new_head_last.next should also != gnull */
		old_head_last = new_head_last;
	}
	LQUEUE_ASSERT(COUNT_GE(old_head_last.count, new_head_last.count));
	goto retry_last_got;
}

static inline __attribute__((__always_inline__))
void lqueue_free_sync(struct lqueue *const q, void *const gnull)
{
	struct raw_lqueue_node head_last;

	head_last.count = atomic_load_explicit(&q->last.count, memory_order_acquire);
	head_last.next = atomic_load_explicit(&q->last.next, memory_order_relaxed);

	if (head_last.next == (uintptr_t)gnull || (head_last.next & NEED_PUSH_FIRST))
		push_first_enqueue(q, head_last);
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
#ifndef LQUEUE_DEBUG
	return lqueue_enqueue_ex(q, new_node, q, NULL, (void *)(uintptr_t)0, default_element_to_node);
#else
	return lqueue_enqueue_ex(q, new_node, (void *)(uintptr_t)((uintptr_t)q | (uintptr_t)(alignof(struct lqueue_node) - 1)),
			NULL, (void *)(uintptr_t)0, default_element_to_node);
#endif
}

struct lqueue_dequeue_ret {
	struct lqueue_node *node;
	bool is_last;
};
static inline __attribute__((__always_inline__))
struct lqueue_dequeue_ret lqueue_dequeue(struct lqueue *const q)
{
#ifndef LQUEUE_DEBUG
	const struct lqueue_dequeue_ex_ret ret = lqueue_dequeue_ex(q, q, NULL, (void *)(uintptr_t)0, default_element_to_node);
#else
	const struct lqueue_dequeue_ex_ret ret = lqueue_dequeue_ex(q, (void *)(uintptr_t)((uintptr_t)q | (uintptr_t)(alignof(struct lqueue_node) - 1)),
			NULL, (void *)(uintptr_t)0, default_element_to_node);
#endif

	return (struct lqueue_dequeue_ret){ret.element, ret.is_last};
}
#endif
