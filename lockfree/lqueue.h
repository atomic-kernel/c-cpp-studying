/*
 * Lockfree queue:
 * 1. The caller must ensure that nodes and heads will never be "free"
 *    For example, let them allocate from the static memory
 * 2. The caller is not allowed to modify the content of the node,
 *    even if it is not in the queue
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
struct tag_pnode {
	union {
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
};

#ifdef __clang__
// May fail on GCC
_Static_assert(__atomic_always_lock_free(sizeof(_Atomic struct __tag_pnode), (void *)(uintptr_t)_Alignof(_Atomic struct __tag_pnode)));
#endif

struct lqueue_node {
	struct tag_pnode next;
};

struct lqueue {
	struct tag_pnode first;
	struct tag_pnode last;
	atomic_size_t num;
	struct lqueue_node dummy;
	bool dummy_is_free;
};

static inline __attribute__((always_inline))
void lqueue_init(struct lqueue *const q)
{
	memset(q, 0, sizeof(*q));
	q->first.raw_p = &q->dummy;
	q->last.raw_p = &q->dummy;
}

// Return whether lqueue is empty before queueing.
static inline __attribute__((always_inline))
bool lqueue_enqueue(struct lqueue *const q, struct lqueue_node *const node)
{
	size_t count;
	struct tag_pnode old_last;
	struct tag_pnode new_last;

	assert(q);
	assert(node);
	assert(node->next.raw_p == NULL);

	while (1) {
		struct tag_pnode old_next;
		struct tag_pnode new_next;

		old_last.raw = atomic_load_explicit(&q->last.atomic, memory_order_acquire);
retry:
		assert(node != old_last.raw_p);
		old_next.raw = atomic_load_explicit(&old_last.raw_p->next.atomic, memory_order_relaxed);

		if (old_next.raw_count != old_last.raw_count) {
			assert(old_next.raw_count > old_last.raw_count);
			continue;
		}
		count = old_last.raw_count;

		if (old_next.raw_p) {
			atomic_thread_fence(memory_order_acquire);
			new_last.raw_p = old_next.raw_p;
			new_last.raw_count = count + 1;
			/* 使用old_last不变保证old_next.raw_p有效 */
			if (atomic_compare_exchange_weak_explicit(&q->last.atomic, &old_last.raw, new_last.raw, memory_order_release, memory_order_acquire))
				old_last.raw = new_last.raw;
			goto retry;
		}

		assert(atomic_load_explicit(&node->next.count, memory_order_relaxed) <= count + 1);
		atomic_store_explicit(&node->next.count, count + 1, memory_order_relaxed);
		new_next.raw_p = node;
		new_next.raw_count = count;
		if (atomic_compare_exchange_weak_explicit(&old_last.raw_p->next.atomic, &old_next.raw, new_next.raw, memory_order_release, memory_order_relaxed))
			break;
	}

	new_last.raw_p = node;
	new_last.raw_count = count + 1;
	atomic_compare_exchange_strong_explicit(&q->last.atomic, &old_last.raw, new_last.raw, memory_order_release, memory_order_relaxed);

	return !atomic_fetch_add_explicit(&q->num, 1, memory_order_release);
}

static inline __attribute__((always_inline))
struct lqueue_node *lqueue_dequeue(struct lqueue *const q, bool *const is_empty_after_dequeue)
{
	size_t num;
	struct tag_pnode old_first;
	struct tag_pnode new_first;
	struct tag_pnode old_last;
	struct tag_pnode new_last;
	struct tag_pnode old_next;
	struct lqueue_node *ret;

	num = atomic_load_explicit(&q->num, memory_order_relaxed);
	do {
		if (!num)
			return NULL;
	} while (!atomic_compare_exchange_weak_explicit(&q->num, &num, num - 1, memory_order_acquire, memory_order_relaxed));

	while (1) {
		old_first.raw = atomic_load_explicit(&q->first.atomic, memory_order_acquire);
		old_last.raw = atomic_load_explicit(&q->last.atomic, memory_order_acquire);
retry1:

		assert(old_first.raw_count <= old_last.raw_count);
		if (old_first.raw_count == old_last.raw_count) {
			assert(old_first.raw_p == old_last.raw_p);
			assert(old_last.raw_p != &q->dummy);
			assert(q->dummy_is_free);

			old_next.raw = atomic_load_explicit(&old_last.raw_p->next.atomic, memory_order_relaxed);
			if (old_next.raw_count != old_last.raw_count) {
				assert(old_next.raw_count > old_last.raw_count);
				continue;
			}
			if (old_next.raw_p) {
				atomic_thread_fence(memory_order_acquire);
				new_last.raw_p = old_next.raw_p;
				new_last.raw_count = old_last.raw_count + 1;
				if (atomic_compare_exchange_weak_explicit(&q->last.atomic, &old_last.raw, new_last.raw, memory_order_release, memory_order_acquire))
					old_last.raw = new_last.raw;
				goto retry1;
			}
			q->dummy_is_free = 0;
			lqueue_enqueue(q, &q->dummy);
			continue;
		}

		new_first.raw_p = old_first.raw_p->next.raw_p;
		new_first.raw_count = old_first.raw_count + 1;
		if (!atomic_compare_exchange_weak_explicit(&q->first.atomic, &old_first.raw, new_first.raw, memory_order_release, memory_order_acquire))
			continue;
		ret = old_first.raw_p;
		struct tag_pnode tmp_old;
		struct tag_pnode tmp_new;
		tmp_old.raw_p = ret->next.raw_p;
		tmp_old.raw_count = new_first.raw_count - 1;
		tmp_new.raw_p = NULL;
		tmp_new.raw_count = new_first.raw_count;
		assert(atomic_compare_exchange_strong_explicit(&ret->next.atomic, &tmp_old.raw, tmp_new.raw, memory_order_relaxed, memory_order_relaxed));

		if (ret != &q->dummy)
			break;
		assert(!q->dummy_is_free);
		q->dummy_is_free = 1;
	}

	*is_empty_after_dequeue = (num == 1);
	return ret;
}

#endif
