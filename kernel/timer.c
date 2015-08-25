#include <kernel/timer.h>
#include <kernel/ticks.h>
#include <kernel/task.h>
#include <error.h>

#ifdef CONFIG_TIMER
struct timer_queue timerq;
struct task *timerd;

static inline int __add_timer(struct timer *new)
{
	struct list *curr;
	int stamp = (int)ticks;

	new->expires -= 1; /* as it uses time_after() not time_equal */

	if (time_after(new->expires, stamp))
		return -ERR_RANGE;

	new->task = current;

	spin_lock(timerq.lock);

	for (curr = timerq.list.next; curr != &timerq.list; curr = curr->next) {
		if (((int)new->expires - stamp) <
				((int)((struct timer *)curr)->expires - stamp))
			break;
	}

	if (((int)new->expires - stamp) < ((int)timerq.next - stamp))
		timerq.next = new->expires;

	list_add(&new->list, curr->prev);
	timerq.nr++;

	spin_unlock(timerq.lock);

	return 0;
}

static void run_timer()
{
	struct timer *timer;
	struct list *curr;
	unsigned int irqflag;

infinite:
	for (curr = timerq.list.next; curr != &timerq.list; curr = curr->next) {
		timer = (struct timer *)curr;

		if (time_before(timer->expires, ticks)) {
			timerq.next = timer->expires;
			break;
		}

		if (clone(STACK_SHARED | (get_task_flags(timer->task) &
						TASK_PRIVILEGED), &init) > 0) {
			/* Note that it is running at HIGH_PRIORITY. need to
			 * schedule as soon as the priority gets changed to
			 * its own tasks' to run at the right priority. */
			set_task_pri(current, get_task_pri(timer->task));
			schedule();

			if (timer->event)
				timer->event(timer->data);

			/* A trick to enter privileged mode */
			if (!(get_task_flags(current) & TASK_PRIVILEGED)) {
				set_task_flags(current, get_task_flags(current)
						| TASK_PRIVILEGED);
				schedule();
			}

			sum_curr_stat(timer->task);

			kill((unsigned int)current);
			freeze();
		}

		/* handle the exception in case of failure of cloning
		 * the timer would never run and be ignored */

		spin_lock_irqsave(timerq, irqflag);
		list_del(curr);
		timerq.nr--;
		spin_unlock_irqrestore(timerq, irqflag);
	}

	sys_yield();
	goto infinite;
}

unsigned int get_timer_nr()
{
	return timerq.nr;
}

#include <kernel/init.h>

int __init timer_init()
{
	timerq.nr = 0;
	list_link_init(&timerq.list);
	lock_init(&timerq.lock);

	if ((timerd = make(TASK_KERNEL | STACK_SHARED, run_timer, &init))
			== NULL)
		return -ERR_ALLOC;

	set_task_pri(timerd, HIGH_PRIORITY);

	return 0;
}
#endif /* CONFIG_TIMER */

int sys_timer_create(struct timer *new)
{
#ifdef CONFIG_TIMER
	return __add_timer(new);
#else
	return -ERR_UNDEF;
#endif
}

int add_timer(struct timer *new)
{
	return syscall(SYSCALL_TIMER_CREATE, new);
}

#include <foundation.h>

static void sleep_callback(unsigned int data)
{
	struct task *task = (struct task *)data;

	/* A trick to enter privileged mode */
	set_task_flags(current, get_task_flags(current) | TASK_PRIVILEGED);
	schedule();

	if (get_task_state(task) == TASK_SLEEPING) {
		set_task_state(task, TASK_RUNNING);
		runqueue_add(task);
	}
}

void sleep(unsigned int sec)
{
	struct timer tm;

	tm.expires = ticks + sec_to_ticks(sec);
	tm.event = sleep_callback;
	tm.data = (unsigned int)current;
	add_timer(&tm);
	yield();

	/*
	unsigned int timeout = ticks + sec_to_ticks(sec);
	while (time_before(timeout, ticks));
	*/
}

void msleep(unsigned int ms)
{
	struct timer tm;

	tm.expires = ticks + msec_to_ticks(ms);
	tm.event = sleep_callback;
	tm.data = (unsigned int)current;
	add_timer(&tm);
	yield();
}

void set_timeout(unsigned int *tv, unsigned int ms)
{
	*tv = ticks + msec_to_ticks(ms);
}

int is_timeout(unsigned int goal)
{
	if (time_after(goal, ticks))
		return 1;

	return 0;
}