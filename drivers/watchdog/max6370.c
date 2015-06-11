#include <common.h>
#include <watchdog.h>
#include <asm/mpc85xx_gpio.h>

void hw_watchdog_reset(void)
{
    	int gpio = 20;
	gpio_direction_output(gpio, 1);
	gpio_direction_output(gpio, 0);
}

void hw_watchdog_init(void)
{
	writeb(0x05, 0xFFA00002);
	printf("wdt status %.8x reset\n", readl(0xFFA00002));
}
