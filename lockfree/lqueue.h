#ifdef LQUEUE_H
#define LQUEUE_H

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

struct lqueue_node {
	struct tag_pnode next;
};

struct lqueue {
	struct tag_pnode first;
	struct tag_pnode tail;
	struct lqueue_note dummy;
	atomic_size_t num;
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
bool lqueue_queue(struct lqueue *const q, struct lqueue_node *const node)
{
	struct tag_pnode old_tail;
	struct tag_pnode new_tail;
	struct tag_pnode old_next;
	struct tag_pnode new_next;

	// flush node->next to NULL
	atomic_fetch_add_explicit(&node->next.count, 1, memory_order_relaxed);
	// 实际上不需要原子，仅为了保证写入顺序 count -> p
	atomic_store_explicit(&node->next.p, NULL, memory_order_release);
	//old_next.raw = node->next.raw;
	//new_next.raw_p = NULL;
	//new_next.count = old_next.raw_count + 1;
	// 实际上可以不使用
	//assert(atomic_compare_exchange_strong_explicit(&node->next.atomic, &old_next.raw, new_next.raw, memory_order_relaxed, memory_order_relaxed));

	do {
		/*
		 * Is memory_order_acquire needed here?
		 * I believe it is necessary because we must ensure that
		 * the previous 'tail->next is written as NULL'
		 * can be visibled in this thread
		 */
		old_tail.raw = atomic_load_explicit(&q->tail.atomic, memory_order_acquire);
retry:
		/*
		 * The user must ensure that the unqueued node can't be free;
		 * otherwise, accessing tail->next is dangerous,
		 * which may cause a segment fault
		 *
		 * The following content is similar, so it will be omitted
		 */
		old_next.raw = atomic_load_explicit(&old_tail.raw_p->next.atomic, memory_order_relaxed);
		if (old_next.raw_p) {
			new_tail.raw_p = old_next.raw_p;
			new_tail.count = old_tail.count + 1;
			/*
			 * Is memory_order_release needed here?
			 * I believe it is necessary because we must ensure that
			 * the previous 'new_tail.raw_p->next is written as NULL'
			 * can be visibled to all threads before 'q->tail updating' can be visibled
			 */
			if (atomic_compare_exchange_weak_explicit(&q->tail.atomic, &old_tail.raw, new_tail.raw, memory_order_release, memory_order_acquire))
				old_tail.raw = new_tail.raw;
			goto retry;
		}

		new_next.raw_p = node;
		new_next.count = old_next.count; // 此处不需要刷新计数器
	} while (!atomic_compare_exchange_weak_explicit(&old_tail.raw_p->next.atomic, &old_next.raw, new_next.raw, memory_order_release, memory_order_relaxed));

	new_tail.raw_p = node;
	new_tail.count = old_tail.count + 1;
	/*
	 * Is memory_order_release needed here?
	 * I believe it is necessary because we must ensure that
	 * the previous 'new_tail.raw_p->next(node->next) is written as NULL'
	 * and 'old_tail.raw_p->next(tail->next) is written as node'
	 * can be visibled to all threads before 'q->tail updating' can be visibled
	 */
	atomic_compare_exchange_strong_explicit(&q->tail.atomic, &old_tail.raw, new_tail.raw, memory_order_release, memory_order_relaxed);

	return !atomic_fetch_add_explicit(&q->num, 1, memory_order_release);
}

static inline __attribute__((always_inline))
struct lqueue_node *lqueue_dequeue(struct lqueue *const q, bool *const is_empty_after_dequeue)
{
	size_t num;
	struct tag_pnode old_first;
	struct tag_pnode new_first;
	struct tag_pnode old_tail;
	struct tag_pnode new_tail;

	num = atomic_load_explicit(&q->num, memory_order_relaxed);
	do {
		if (!num)
			return NULL;
	} while (!atomic_compare_exchange_weak_explicit(&q->num, &num, num - 1, memory_order_acquire, memory_order_relaxed));

	while (1) {
		// This acquire is sync for the queueer
		old_tail.raw = atomic_load_explicit(&q->tail.atomic, memory_order_acquire);
retry1:
		// This acquire is sync for other dequeueer
		old_first.raw = atomic_load_explicit(&q->first.atomic, memory_order_acquire);

		if (old_first.raw_p == old_tail.raw_p) {
			old_next_p = atomic_load_explicit(&old_tail.raw_p->next.p, memory_order_relaxed);
			if (old_next_p) {
				new_tail.raw_p = old_next.raw_p;
				new_tail.count = old_tail.count + 1;
				if (atomic_compare_exchange_weak_explicit(&q->tail.atomic, &old_tail.raw, new_tail.raw, memory_order_release, memory_order_acquire))
					old_tail.raw = new_tail.raw;
				goto retry1;
			} else {
				// 插入 dummy
				// 重试
				assert(old_tail.raw_p != &p->dummy);
				assert(q->dummy_free);
				q->dummy_free = 0;
				lqueue_queue(q, &q->dummy);
				continue;
			}
		}

		new_first.raw_p = old_first.raw_p->next.raw_p;
		new_first.raw_count = old_first.raw_count + 1;
		if (!atomic_compare_exchange_weak_explicit(&q->first.atomic, &old_first.raw, new_first.raw, memory_order_release, memory_order_acquire))
			continue;
		if (old_first.raw_p != &q->dummy)
			break;
		atomic_set_explicit(&q->dummy_busy, 0, memory_order_release);
	}

	*is_empty_after_dequeue = (num == 1);
	return old_first.raw_p;
}

#endif
