#include <foundation.h>
#include <kernel/init.h>
#include <kernel/task.h>
#include <kernel/sched.h>

static void __init load_user_task()
{
	extern char _user_task_list;
	struct task *p;
	unsigned int pri;
	size_t stack_size;

	for (p = (struct task *)&_user_task_list; *(unsigned int *)p; p++) {
		if (p->addr == NULL)
			continue;

		/* task size including heap and stack */
		stack_size = (size_t)p->size;

		/* share the init kernel stack to save memory */
		if (alloc_mm(p, stack_size, STACK_SHARED, &init))
			continue;

		pri = get_task_pri(p);
		set_task_dressed(p, p->flags | STACK_SHARED, p->addr);
		set_task_pri(p, pri);
		set_task_context(p, wrapper);

		/* make it runnable, and add into runqueue */
		set_task_state(p, TASK_RUNNING);
		runqueue_add_core(p);
	}
}

static int __init make_init_task()
{
	extern void idle(); /* becomes init task */

	/* stack must be allocated first. and to build root relationship
	 * properly `current` must be set to `init`. */
	current = &init;

	if (alloc_mm(&init, STACK_SIZE_MIN, 0, NULL))
		return ENOMEM;

	set_task_dressed(&init, TASK_STATIC | TASK_KERNEL, idle);
	set_task_context_hard(&init, wrapper);
	set_task_state(&init, TASK_RUNNING);
	init.name = "idle";

	/* make it the sole */
	links_init(&init.children);
	links_init(&init.sibling);

	return 0;
}

#include <fs/fs.h>
#include <kernel/device.h>

static int __init console_init()
{
	extern int sys_open_core(char *filename, int mode, void *opt);

	stdin = stdout = stderr =
		sys_open_core(DEVFS_ROOT CONSOLE, O_RDWR, NULL);

	return 0;
}

static void __init sys_init()
{
	extern char _init_func_list;
	unsigned int *p = (unsigned int *)&_init_func_list;

	while (*p)
		((void (*)())*p++)();
}

#include <kernel/page.h>
#include <kernel/softirq.h>
#include <kernel/timer.h>
#include <kernel/systick.h>
#include <asm/power.h>
#include <bitops.h>

int f_reset;

static const char *str_reset[] = {
	"RMVF",
	"Brownout",
	"System",
	"Power",
	"Software",
	"Independent watchdog",
	"Window watchdog",
	"Low-power",
};

extern void mm_init();
extern void fs_init();
extern void driver_init();
extern void device_init();
extern void systick_init();
extern void scheduler_init();
extern int timer_init();
extern void debug_init();

int __init kernel_init()
{
	/* keep the calling order below because of dependencies */
	sys_init();
	mm_init();
	fs_init();

	make_init_task();
	softirq_init();

	driver_init();
	device_init();
	systick_init();
	scheduler_init();
	console_init();

	load_user_task(); /* that are registered statically */

#ifdef CONFIG_TIMER
	timer_init();
#endif
#ifdef CONFIG_DEBUG
	debug_init();
#endif

	f_reset = __read_reset_source();

	/* a banner */
	notice("\n\nyaos %s %s\n"
	       "[%08x] reset by %s(%x)",
	       def2str(VERSION), def2str(MACHINE),
	       get_sysclk(), str_reset[fls(f_reset)-1], f_reset);
	notice("[%08x] systick: %dHz, sysclk:%dHz\n"
	       "[%08x] hclk: %dHz, pll: %dHz, pclk1: %dHz, plck2 %dHz\n"
	       "[%08x] adclk: %dHz",
	       get_sysclk(), sysfreq, get_sysclk_freq(),
	       get_sysclk(), get_hclk(), get_pllclk(), get_pclk1(), get_pclk2(),
	       get_sysclk(), get_adclk());
	notice("[%08x] interrupt %s",
	       get_sysclk(), is_interrupt_disabled()? "disabled" : "enabled");
	extern char _ram_size, _rom_size;
	notice("[%08x] ram size: %d, rom size: %d",
	       get_sysclk(), (int)&_ram_size, (int)&_rom_size);

	/* switch from boot stack memory to new one */
	set_user_sp(init.mm.sp);
	set_kernel_sp(init.mm.kernel.sp);

	/* everything ready now */
#ifndef ARMv7A
	sei();
#endif
	resched();

	/* it doesn't really reach up to this point. init task becomes idle as
	 * its context is already set to idle */
	__context_restore(current);
	__ret_from_exc(0);
	freeze();

	return 0;
}
