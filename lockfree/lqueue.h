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
struct raw_tag_pnode {
	struct lqueue_node *p;
	uintptr_t count;
};
union tag_pnode {
	struct {
		union {
			struct lqueue_node *raw_p;
			_Atomic(struct lqueue_node *) p;
		};
		union {
			uintptr_t raw_count;
			atomic_uintptr_t count;
		};
	};
	struct raw_tag_pnode raw;
	_Atomic struct raw_tag_pnode atomic;
};

struct lqueue_node {
	union tag_pnode next;
};
struct raw_lqueue {
	union tag_pnode first;
	union tag_pnode last;
};
struct lqueue {
	struct raw_lqueue raw_q;
	atomic_size_t num;
	struct lqueue_node dummy;
	bool dummy_is_free;
};

#ifndef UINTPTR_MAX
#error "UINTPTR_MAX not defined"
#endif
#if UINTPTR_MAX == UINT64_MAX /* 64bits */
_Static_assert(sizeof(union tag_pnode) == 16 && sizeof(void *) == 8 &&
	       _Alignof(union tag_pnode) == 16, "size check failed!");
# ifndef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
#  warning "your platform may be not support atomic 16"
# elif defined(__clang__) // May failed on gcc
_Static_assert(__atomic_always_lock_free(sizeof(_Atomic struct raw_tag_pnode),
			(void *)(uintptr_t)_Alignof(_Atomic struct raw_tag_pnode)),
		"lock free check failed\n");
# endif
#elif UINTPTR_MAX == UINT32_MAX /* 32bits */
_Static_assert(sizeof(union tag_pnode) == 8 && sizeof(void *) == 4 &&
	       _Alignof(union tag_pnode) == 8, "size check failed!");
# ifndef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
#  warning "your platform may be not support atomic 8"
# else
_Static_assert(__atomic_always_lock_free(sizeof(_Atomic struct raw_tag_pnode),
			(void *)(uintptr_t)_Alignof(_Atomic struct raw_tag_pnode)),
		"lock free check failed\n");
# endif
#else /* unknown bits */
# error "unknown wordsize"
#endif

static inline __attribute__((always_inline))
void raw_lqueue_init(struct raw_lqueue *const q, struct lqueue_node *const first)
{
	assert((void *)q != first);
	/*
	 * use q as last->next instead of NULL,
	 * This is to handle ABA issues that may arise between multiple queues
	 */
	first->next.raw_p = (void *)q;
	q->first.raw_count = q->last.raw_count = first->next.raw_count = 0;
	q->first.raw_p = q->last.raw_p = first;
}
static inline __attribute__((always_inline))
void lqueue_init(struct lqueue *const q)
{
	raw_lqueue_init(&q->raw_q, &q->dummy);
	q->num = 0;
	q->dummy_is_free = false;
}

static inline __attribute__((always_inline))
void raw_lqueue_enqueue(struct raw_lqueue *const q, struct lqueue_node *const node)
{
	uintptr_t old_last_count;
	struct raw_tag_pnode raw_old_last;
	struct raw_tag_pnode raw_new_last;
	struct raw_tag_pnode raw_old_next;
	struct raw_tag_pnode raw_new_next;

	assert(node != (void *)q);
	atomic_store_explicit(&node->next.p, NULL, memory_order_relaxed);

retry0:
	raw_old_last.count = atomic_load_explicit(&q->last.count, memory_order_acquire);
	// 必须保证，先读count，再读p，以保证新增正确
	raw_old_last.p = atomic_load_explicit(&q->last.p, memory_order_relaxed);
retry1:
	assert(node != raw_old_last.p);
	// 必须保证先读取count，再读取next，以保证推进正确
	raw_old_next.p = atomic_load_explicit(&raw_old_last.p->next.p, memory_order_relaxed);
	old_last_count = raw_old_last.count;

	if (unlikely(raw_old_next.p != (void *)q)) {
		/* acquire with load last->next */
		atomic_thread_fence(memory_order_acquire);
		raw_new_last.p = raw_old_next.p;
		raw_new_last.count = old_last_count + 1;
		if (atomic_compare_exchange_weak_explicit(&q->last.atomic, &raw_old_last, raw_new_last, memory_order_release, memory_order_acquire)) {
			/* not NULL or (void *)-1 */
			assert((uintptr_t)raw_new_last.p + 1 > 1);
			raw_old_last = raw_new_last;
		}
		goto retry1;
	}

	/*
	 * Guarantee the writing order:
	 * node->next.p = NULL
	 * node->next.count = new_count
	 * node->next.p = q
	 */ 
	atomic_store_explicit(&node->next.count, old_last_count + 1, memory_order_release);
	atomic_store_explicit(&node->next.p, (void *)q, memory_order_release);
	raw_new_next.count = raw_old_next.count = old_last_count;
	raw_new_next.p = node;
	if (unlikely(!atomic_compare_exchange_weak_explicit(&raw_old_last.p->next.atomic, &raw_old_next, raw_new_next, memory_order_release, memory_order_relaxed)))
		goto retry0;

	raw_new_last.p = node;
	raw_new_last.count = old_last_count + 1;
	// 用strong防止返回虚假的空
	if (unlikely(!atomic_compare_exchange_strong_explicit(&q->last.atomic, &raw_old_last, raw_new_last, memory_order_release, memory_order_relaxed))) {
		if (raw_old_last.count == raw_new_last.count)
			assert(raw_old_last.p == raw_new_last.p);
	}
}

// Return whether lqueue is empty before queueing.
static inline __attribute__((always_inline))
bool lqueue_enqueue(struct lqueue *const q, struct lqueue_node *const node)
{
	assert(node != &q->dummy);
	raw_lqueue_enqueue(&q->raw_q, node);
	return !atomic_fetch_add_explicit(&q->num, 1, memory_order_release);
}

static inline __attribute__((always_inline))
struct lqueue_node *raw_lqueue_dequeue(struct raw_lqueue *const q)
{
	struct lqueue_node *next;
	struct raw_tag_pnode raw_old_first;
	struct raw_tag_pnode raw_new_first;
	struct raw_tag_pnode raw_old_last;

	// 此处其实用relaxed就可以了，主要是为了防止返回虚假的空
	raw_old_first.count = atomic_load_explicit(&q->first.count, memory_order_acquire);
	// 保证old_first.p读取在old_firtst.count之后，可以方便编码，实际非必须
	raw_old_first.p = atomic_load_explicit(&q->first.p, memory_order_relaxed);
retry0:
	// 1. 必须保证先读取last.count，再读取last->next，以保证推进正确性
	// 2. 处理来自enqueue的release
	raw_old_last.count = atomic_load_explicit(&q->last.count, memory_order_acquire);
retry1:
	// 对于推进来说，需要原子读取
	next = atomic_load_explicit(&raw_old_first.p->next.p, memory_order_relaxed);

	assert(raw_old_first.count <= raw_old_last.count);
	if (raw_old_first.count == raw_old_last.count) {
		struct raw_tag_pnode raw_new_last;

		if (likely(next == (void *)q))
			return NULL;

		atomic_thread_fence(memory_order_acquire); // acquire with load next
		raw_old_last.p = raw_old_first.p;
		raw_new_last.p = next;
		raw_new_last.count = raw_old_last.count + 1;
		if (atomic_compare_exchange_weak_explicit(&q->last.atomic, &raw_old_last, raw_new_last, memory_order_release, memory_order_acquire)) {
			assert((uintptr_t)raw_new_last.p + 1 > 1);
			goto label_continue;
		} else if (raw_old_last.count == raw_old_first.count) {
			assert(raw_old_last.p == raw_old_first.p);
		}
		goto retry1;
	}

label_continue:
	raw_new_first.p = next;
	raw_new_first.count = raw_old_first.count + 1;
	// We need release when success because we need to make sure that writing q->last can be seen.
	if (unlikely(!atomic_compare_exchange_weak_explicit(&q->first.atomic, &raw_old_first, raw_new_first, memory_order_release, memory_order_acquire)))
		goto retry0;
	atomic_store_explicit(&raw_old_first.p->next.p, (void *)-1, memory_order_relaxed); // for debug
	assert((uintptr_t)raw_new_first.p + 1 > 1);
	assert(raw_new_first.p != (void *)q);
	return raw_old_first.p;
}

static inline __attribute__((always_inline))
struct lqueue_node *lqueue_dequeue(struct lqueue *const q, bool *const is_empty_after_dequeue)
{
	size_t num;
	struct lqueue_node *ret;

	num = atomic_load_explicit(&q->num, memory_order_relaxed);
	do {
		if (!num)
			return NULL;
	} while (unlikely(!atomic_compare_exchange_weak_explicit(&q->num, &num, num - 1, memory_order_acquire, memory_order_relaxed)));

	*is_empty_after_dequeue = (num == 1);

	while (1) {
		ret = raw_lqueue_dequeue(&q->raw_q);
		if (likely(ret)) {
			if (likely(ret != &q->dummy))
				return ret;
			assert(!q->dummy_is_free);
			q->dummy_is_free = 1;
			continue;
		}
		assert(q->dummy_is_free);
		q->dummy_is_free = 0;
		raw_lqueue_enqueue(&q->raw_q, &q->dummy);
	}
	__builtin_unreachable();
}
#endif
