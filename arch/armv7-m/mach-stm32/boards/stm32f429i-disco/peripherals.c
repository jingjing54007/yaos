#include <kernel/module.h>
#include <pinmap.h>

REGISTER_DEVICE(uart, "uart", 1);
REGISTER_DEVICE(gpio, "led", PIN_LED_GREEN);
REGISTER_DEVICE(gpio, "led", PIN_LED_RED);

void clock_init()
{
	extern void SystemClock_Config(void);
	SystemClock_Config();
}
