#ifndef __LOCK_H__
#define __LOCK_H__

#include <types.h>
#include <kernel/waitqueue.h>

struct semaphore {
	lock_t counter;
	struct waitqueue_head wq;
};

typedef struct semaphore mutex_t;

#ifdef MACHINE
#include <asm/lock.h>
#endif

/* the name of `counter` implies referenced by pointer while the name of
 * `count` implies referenced by value. */
#define DEFINE_LOCK(name)		lock_t name = UNLOCKED
#define lock_init(couter)		(*(lock_t *)(couter) = UNLOCKED)
#define is_locked(count)		((count) <= 0)

#define lock_atomic(counter)		__lock_atomic(counter)
#define unlock_atomic(counter)		__unlock_atomic(counter)

/* spinlock */
#ifdef CONFIG_SMP
#define DEFINE_SPINLOCK(name)		DEFINE_LOCK(name)
#define spin_lock(counter)		lock_atomic(counter)
#define spin_unlock(counter)		unlock_atomic(counter)
#else
#define DEFINE_SPINLOCK(name)
#define spin_lock(counter)
#define spin_unlock(counter)
#endif /* CONFIG_SMP */

#include <kernel/interrupt.h>

#define spin_lock_irqsave(counter, flag)	({			\
	irq_save(flag);							\
	local_irq_disable();						\
	spin_lock(counter);						\
})

#define spin_unlock_irqrestore(counter, flag)	({			\
	spin_unlock(counter);						\
	irq_restore(flag);						\
})

#include <kernel/waitqueue.h>

#define DEFINE_SEMAPHORE(name, v)					\
	struct semaphore name = {					\
		.counter = v,						\
		.wq = INIT_WAIT_HEAD(name.wq),				\
	}

#define INIT_SEMAPHORE(name, v)	name = (struct semaphore){		\
	.counter = v,							\
	.wq = INIT_WAIT_HEAD(name.wq),					\
}

#define sem_init(sem, c) ({						\
	((struct semaphore *)(sem))->counter = c;			\
	((struct semaphore *)(sem))->wq = (struct waitqueue_head)	\
		INIT_WAIT_HEAD(((struct semaphore *)(sem))->wq);	\
})
#define sem_dec(sem)			__semaphore_dec(sem, INF)
#define sem_inc(sem)			__semaphore_inc(sem)
#define sem_wait(sem, ms)		__semaphore_dec_wait(sem, ms)
#define sem_wake(sem)			__semaphore_inc(sem)

#include <kernel/sched.h>

/* mutex */
#define DEFINE_MUTEX(name)		DEFINE_SEMAPHORE(name, 1)
#define INIT_MUTEX(name)		INIT_SEMAPHORE(name, 1)
#define mutex_lock(sem)			__semaphore_dec(sem, INF)
#define mutex_unlock(sem)		__semaphore_inc(sem)
#define mutex_lock_atomic(sem)		\
	lock_atomic(&((mutex_t *)(sem))->counter)
#define mutex_unlock_atomic(sem)	\
	unlock_atomic(&((mutex_t *)(sem))->counter)
#define mutex_init(sem) ({						\
	((mutex_t *)(sem))->counter = 1;				\
	((mutex_t *)(sem))->wq = (struct waitqueue_head)		\
		INIT_WAIT_HEAD(((mutex_t *)(sem))->wq);			\
})

/* reader-writer spin lock */
#define DEFINE_RWLOCK(name)		DEFINE_LOCK(name)
#define read_lock(counter)		read_lock_spinning(counter)
#define read_unlock(counter)		atomic_sub(1, counter)
#define write_lock(counter)		write_lock_spinning(counter)
#define write_unlock(counter)		atomic_add(1, counter)

#endif /* __LOCK_H__ */
