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
#include <stdbool.h>
#include <string.h>
#include <assert.h>

struct lqueue_node;
struct __tag_pnode {
	struct lqueue_node *p;
	size_t count;
};
union tag_pnode {
	struct {
		union {
			struct lqueue_node *raw_p;
			_Atomic(struct lqueue_node *) p;
		};
		union {
			size_t raw_count;
			atomic_size_t count;
		};
	};
	struct __tag_pnode raw;
	_Atomic struct __tag_pnode atomic;
};

#ifndef UINTPTR_MAX
#error "UINTPTR_MAX not defined"
#endif

#if UINTPTR_MAX == UINT64_MAX /* 64bits */
_Static_assert(sizeof(union tag_pnode) == 16 && sizeof(void *) == 8 &&
	       _Alignof(union tag_pnode) == 16, "size check failed!");
#ifndef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
#warning "your platform may be not support atomic 16"
#elif defined(__clang__) // May failed on gcc
_Static_assert(__atomic_always_lock_free(sizeof(_Atomic struct __tag_pnode),
			(void *)(uintptr_t)_Alignof(_Atomic struct __tag_pnode)),
		"lock free check failed\n");
#endif
#elif UINTPTR_MAX == UINT32_MAX /* 32bits */
_Static_assert(sizeof(union tag_pnode) == 8 && sizeof(void *) == 4 &&
	       _Alignof(union tag_pnode) == 8, "size check failed!");
#ifndef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
#warning "your platform may be not support atomic 8"
#else
_Static_assert(__atomic_always_lock_free(sizeof(_Atomic struct __tag_pnode),
			(void *)(uintptr_t)_Alignof(_Atomic struct __tag_pnode)),
		"lock free check failed\n");
#endif
#else /* unknown bits */
#error "unknown wordsize"
#endif

#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif

struct lqueue_node {
	union tag_pnode next;
};
struct lqueue {
	union tag_pnode first;
	union tag_pnode last;
	atomic_size_t num;
	struct lqueue_node dummy;
	bool dummy_is_free;
};

static inline __attribute__((always_inline))
void __lqueue_init(struct lqueue *const q, struct lqueue_node *const first)
{
	memset(q, 0, sizeof(*q));
	/*
	 * use q as last->next instead of NULL,
	 * This is to handle ABA issues that may arise between multiple queues
	 */
	first->next.raw_p = (void *)q;
	first->next.raw_count = 0;
	q->first.raw_p = first;
	q->last.raw_p = first;
}
static inline __attribute__((always_inline))
void lqueue_init(struct lqueue *const q)
{
	__lqueue_init(q, &q->dummy);
}

static inline __attribute__((always_inline))
void __lqueue_enqueue(struct lqueue *const q, struct lqueue_node *const node)
{
	size_t count;
	union tag_pnode old_last;
	union tag_pnode new_last;

	assert(node != (void *)q);
	atomic_store_explicit(&node->next.p, NULL, memory_order_relaxed);
	atomic_thread_fence(memory_order_release);

	while (1) {
		union tag_pnode old_next;
		union tag_pnode new_next;

		old_last.raw_count = atomic_load_explicit(&q->last.count, memory_order_acquire);
		old_last.raw_p = atomic_load_explicit(&q->last.p, memory_order_relaxed);
retry:
		assert(node != old_last.raw_p);
		old_next.raw_p = atomic_load_explicit(&old_last.raw_p->next.p, memory_order_relaxed);
		old_next.raw_count = atomic_load_explicit(&old_last.raw_p->next.count, memory_order_relaxed);

		if (unlikely(old_next.raw_count != old_last.raw_count))
			continue;
		count = old_last.raw_count;

		if (unlikely(old_next.raw_p != (void *)q)) {
			/* acquire with load last->next */
			atomic_thread_fence(memory_order_acquire);
			new_last.raw_p = old_next.raw_p;
			new_last.raw_count = count + 1;
			if (atomic_compare_exchange_weak_explicit(&q->last.atomic, &old_last.raw, new_last.raw, memory_order_release, memory_order_acquire)) {
				/* not NULL or (void *)-1 */
				assert((uintptr_t)new_last.raw_p + 1 > 1);
				old_last.raw = new_last.raw;
			}
			goto retry;
		}

		/*
		 * Guarantee the writing order:
		 * node->next.p = NULL
		 * node->next.count = new_count
		 * node->next.p = q
		 */ 
		atomic_store_explicit(&node->next.count, count + 1, memory_order_relaxed);
		atomic_store_explicit(&node->next.p, (struct lqueue_node *)q, memory_order_release);
		new_next.raw_p = node;
		new_next.raw_count = count;
		if (likely(atomic_compare_exchange_weak_explicit(&old_last.raw_p->next.atomic, &old_next.raw, new_next.raw, memory_order_release, memory_order_relaxed)))
			break;
	}

	new_last.raw_p = node;
	new_last.raw_count = count + 1;
	if (unlikely(!atomic_compare_exchange_strong_explicit(&q->last.atomic, &old_last.raw, new_last.raw, memory_order_release, memory_order_relaxed))) {
		if (old_last.raw_count == new_last.raw_count)
			assert(old_last.raw_p == new_last.raw_p);
	}
}

// Return whether lqueue is empty before queueing.
static inline __attribute__((always_inline))
bool lqueue_enqueue(struct lqueue *const q, struct lqueue_node *const node)
{
	assert(node != &q->dummy);
	__lqueue_enqueue(q, node);
	return !atomic_fetch_add_explicit(&q->num, 1, memory_order_release);
}

static inline __attribute__((always_inline))
struct lqueue_node *__lqueue_dequeue(struct lqueue *const q)
{
	union tag_pnode old_first;
	union tag_pnode new_first;
	union tag_pnode old_last;

retry0:
	old_first.raw = atomic_load_explicit(&q->first.atomic, memory_order_acquire);
retry1:
	old_last.raw = atomic_load_explicit(&q->last.atomic, memory_order_acquire);
retry2:
	assert(old_first.raw_count <= old_last.raw_count);
	if (old_first.raw_count == old_last.raw_count) {
		union tag_pnode old_next;
		union tag_pnode new_last;

		assert(old_first.raw_p == old_last.raw_p);

		old_next.raw = atomic_load_explicit(&old_last.raw_p->next.atomic, memory_order_relaxed);
		if (unlikely(old_next.raw_count != old_last.raw_count))
			goto retry0;
		if (unlikely(old_next.raw_p != (void *)q)) {
			atomic_thread_fence(memory_order_acquire);
			new_last.raw_p = old_next.raw_p;
			new_last.raw_count = old_last.raw_count + 1;
			if (atomic_compare_exchange_weak_explicit(&q->last.atomic, &old_last.raw, new_last.raw, memory_order_release, memory_order_acquire)) {
				assert((uintptr_t)new_last.raw_p + 1 > 1);
				old_last.raw = new_last.raw;
			}
			goto retry2;
		}
		return NULL;
	}

	new_first.raw_p = old_first.raw_p->next.raw_p;
	new_first.raw_count = old_first.raw_count + 1;
	if (unlikely(!atomic_compare_exchange_weak_explicit(&q->first.atomic, &old_first.raw, new_first.raw, memory_order_release, memory_order_acquire)))
		goto retry1;
	assert((uintptr_t)new_first.raw_p + 1 > 1);
	assert(new_first.raw_p != (void *)q);
	return old_first.raw_p;
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

	while (1) {
		ret = __lqueue_dequeue(q);
		if (likely(ret)) {
			if (likely(ret != &q->dummy))
				break;
			assert(!q->dummy_is_free);
			q->dummy_is_free = 1;
			continue;
		}
		assert(q->dummy_is_free);
		q->dummy_is_free = 0;
		__lqueue_enqueue(q, &q->dummy);
	}
	*is_empty_after_dequeue = (num == 1);
	return ret;
}
#endif
