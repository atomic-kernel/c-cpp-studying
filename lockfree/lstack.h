/*
 * Lockfree stack:
 * The caller must ensure that nodes and heads will never be "free"
 * For example, let them allocate from the static memory
 */

#ifndef LSTACK_H
#define LSTACK_H

#include <stdatomic.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>

struct lstack_node {
	struct lstack_node *next;
};

struct __lstack_head {
	size_t count;
	struct lstack_node *first;
};
struct lstack_head {
	union {
		struct {
			union {
				size_t raw_count;
				atomic_size_t count;
			};
			union {
				struct lstack_node *raw_first;
				_Atomic(struct lstack_node *) first;
			};
		};
		struct __lstack_head raw;
		_Atomic struct __lstack_head atomic;
	};
};

#ifdef __clang__
// May fail on GCC
_Static_assert(__atomic_always_lock_free(sizeof(struct __lstack_head), (void *)(uintptr_t)_Alignof(struct __lstack_head)));
#endif

// Return whether lstack is empty before pushing.
static inline __attribute__((always_inline))
bool lstack_push(struct lstack_head *const head, struct lstack_node *const node)
{
	_Atomic(struct lstack_node *) *pfirst = &head->first;
	struct lstack_node *old_first = atomic_load_explicit(pfirst, memory_order_relaxed);

	do {
		node->next = old_first;
	} while (!atomic_compare_exchange_weak_explicit(pfirst, &old_first, node, memory_order_release, memory_order_relaxed));

	return !old_first;
}

// Return whether lstack is empty after popping.
static inline __attribute__((always_inline))
struct lstack_node *lstack_pop(struct lstack_head *const head, bool *const is_empty_after_pop)
{
	struct lstack_head old_head;
	struct lstack_head new_head;

	old_head.raw = atomic_load_explicit(&head->atomic, memory_order_acquire);
	do {
		if (!old_head.raw_first)
			return NULL;

		/*
		 * The user must ensure that the popped node can't be free;
		 * otherwise, accessing first->next is dangerous,
		 * which may cause a segment fault
		 */
		new_head.raw_first = old_head.raw_first->next;
		new_head.raw_count = old_head.raw_count + 1;
		/*
		 * Can we omit the "acquire" barrier in the case of success?
		 * I think it's OK, because we have already acquired "head" before.
		 * The release barrier is to ensure that
		 * the reading of `old_head->next` is completed
		 * before updating head
		 */
	} while (!atomic_compare_exchange_weak_explicit(&head->atomic, &old_head.raw, new_head.raw, memory_order_release, memory_order_acquire));

	*is_empty_after_pop = !new_head.raw_first;
	return old_head.raw_first;
}
#endif
