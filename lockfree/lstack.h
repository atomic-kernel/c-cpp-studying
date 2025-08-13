/*
 * Lock-free stack:
 * Allow multiple producers and consumers.
 * initialization: memset the stack head to 0 is ok
 * lstack_push(): pushing a node to the lock-free stack
 * lstack_pop(): try popping a node from the lock-free stack
 * lstack_pop_all(): popping all nodes from the lock-free stack
 *
 * constraint:
 * When nodes are popped from a lock-free stack(including lstack_pop and
 * lstack_pop_all operations), they cannot be free, unless it can be guaranteed
 * that all the popping operations of other threads on this lock-free stack
 * before this moment(lstack_pop() func returns) have been completed.
 *
 * If dynamically freeing the nodes is required, something similar to RCU
 * may be needed. It is recommended to reserve the memory of nodes, such as
 * defining them as static or global variables, or allocating the memory of
 * all nodes when initializing a lock-free stack,
 * and freeing all nodes when destroying the lock-free stack.
 */

#ifndef LSTACK_H
#define LSTACK_H

#include <stdatomic.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>
#include <assert.h>

struct lstack_node {
	struct lstack_node *next;
};
struct raw_lstack_head {
	struct lstack_node *first;
	uintptr_t count;
};
struct lstack_head {
	union {
		struct {
			union {
				struct lstack_node *raw_first;
				_Atomic(struct lstack_node *) first;
			};
			union {
				uintptr_t raw_count;
				atomic_uintptr_t count;
			};
		};
		struct raw_lstack_head raw;
		_Atomic struct raw_lstack_head atomic;
	};
};

#ifndef UINTPTR_MAX
#error "UINTPTR_MAX not defined"
#endif

#if UINTPTR_MAX == UINT64_MAX /* 64bits */
_Static_assert(sizeof(struct lstack_head) == 16 && sizeof(void *) == 8 &&
	       _Alignof(struct lstack_head) == 16, "size check failed!");
#ifndef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
#warning "your platform may be not support atomic 16"
#elif defined(__clang__) // May failed on gcc
_Static_assert(__atomic_always_lock_free(sizeof(_Atomic struct raw_lstack_head),
			(void *)(uintptr_t)_Alignof(_Atomic struct raw_lstack_head)),
		"lock free check failed\n");
#endif
#elif UINTPTR_MAX == UINT32_MAX /* 32bits */
_Static_assert(sizeof(struct lstack_head) == 8 && sizeof(void *) == 4 &&
	       _Alignof(struct lstack_head) == 8, "size check failed!");
#ifndef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
#warning "your platform may be not support atomic 8"
#else
_Static_assert(__atomic_always_lock_free(sizeof(_Atomic struct raw_lstack_head),
			(void *)(uintptr_t)_Alignof(_Atomic struct raw_lstack_head)),
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

// Return whether lstack is empty before pushing.
static inline __attribute__((always_inline))
bool lstack_push(struct lstack_head *const head, struct lstack_node *const node)
{
	struct lstack_node *old_first = atomic_load_explicit(&head->first, memory_order_relaxed);

	do {
		node->next = old_first;
	} while (unlikely(!atomic_compare_exchange_weak_explicit(&head->first, &old_first, node, memory_order_release, memory_order_relaxed)));

	return !old_first;
}

// Return whether lstack is empty after popping.
static inline __attribute__((always_inline))
struct lstack_node *lstack_pop(struct lstack_head *const head, bool *const is_empty_after_pop)
{
	struct raw_lstack_head raw_old_head;
	struct raw_lstack_head raw_new_head;

	raw_old_head.count = atomic_load_explicit(&head->count, memory_order_acquire);
	raw_old_head.first = atomic_load_explicit(&head->first, memory_order_acquire);
	do {
		if (!raw_old_head.first)
			return NULL;

		/*
		 * The user must ensure that the popped node can't be free;
		 * otherwise, accessing first->next is dangerous,
		 * which may cause a segment fault
		 */
		raw_new_head.first = raw_old_head.first->next;
		raw_new_head.count = raw_old_head.count + 1;
		/*
		 * Actually, we only need relaxed when success, but c standard
		 * say: fail cannot specify stronger ordering than succ
		 */
	} while (unlikely(!atomic_compare_exchange_weak_explicit(&head->atomic, &raw_old_head, raw_new_head, memory_order_acquire, memory_order_acquire)));

	*is_empty_after_pop = !raw_new_head.first;
	return raw_old_head.first;
}

static inline __attribute__((always_inline))
struct lstack_node *lstack_pop_all(struct lstack_head *const head)
{
	struct raw_lstack_head raw_old_head;
	struct raw_lstack_head raw_new_head;

	raw_old_head.first = atomic_load_explicit(&head->first, memory_order_relaxed);
	raw_old_head.count = atomic_load_explicit(&head->count, memory_order_relaxed);
	do {
		if (!raw_old_head.first)
			return NULL;

		raw_new_head.first = NULL;
		raw_new_head.count = raw_old_head.count + 1;
	} while (unlikely(!atomic_compare_exchange_weak_explicit(&head->atomic, &raw_old_head, raw_new_head, memory_order_acquire, memory_order_relaxed)));

	return raw_old_head.first;
}
#endif
