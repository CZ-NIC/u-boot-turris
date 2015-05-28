/*
 * Copyright 2015
 * CZ.NIC z.s.p.o.
 * Martin Strbacka (martin.strbacka@nic.cz)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <booke_watchdog.h>

static int do_booke_wdt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    	unsigned long timeout;
	const char *param;
	char *endp;

	if (argc != 2)
	    goto err;

	param = argv[1];
	timeout = simple_strtoul(param, &endp, 0);

	if (strcmp(param,"ping") == 0) {
		WATCHDOG_RESET();
	} else if(endp == param) {
		printf("Error: Could not convert `%s' to a number\n\n", param);
		goto err;
	} else if (timeout < 1) {
	    	printf("Error: zero timeouts are invalid\n\n");
		goto err;
	} else if (timeout > 1) {
		booke_wdt_set((unsigned int)timeout);
		WATCHDOG_RESET();
	} else {
		goto err;
	}

	return 0;
err:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	booke_wdt,	2,		0,	do_booke_wdt,
	"Controls booke watchdog",
	""
);
