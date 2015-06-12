/*
 * Copyright 2015
 * CZ.NIC z.s.p.o.
 * Martin Strbacka (martin.strbacka@nic.cz)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

static int do_max6370_wdt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	writeb(0x03, 0xFFA00002);
	printf("wdt status %.8x\n", readl(0xFFA00002));			

	return 0;
}

U_BOOT_CMD(
	max6370_wdt_off,	1,		1,	do_max6370_wdt,
	"Turns off MAX6370 watchdog",
	""
);
