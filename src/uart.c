// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 RnD Center "ELVEES", JSC
 */

#include <common.h>
#include <regs.h>
#include <plat/plat.h>
#include <uart.h>

#define UART_LCR_DEFAULT 3 /* 8 bit, no parity, 1 stop bit. */

void uart_cfg(void)
{
	uint16_t divisor;

	platform_uart_cfg();

	/* Set baudrate */
	divisor = CONFIG_UART_BASE_FREQ / (CONFIG_UART_BAUDRATE * 16);
	write32(UART_LCR, UART_LCR_DEFAULT | UART_LCR_DLAB);
	write32(UART_DLH, divisor >> 8);
	write32(UART_DLL, divisor);
	write32(UART_LCR, UART_LCR_DEFAULT);

	write32(UART_IER, 0);
	write32(UART_MCR, UART_MCR_DTR | UART_MCR_RTS);
}

int putchar(int c)
{
	while (1) {
		while (!(read32(UART_LSR) & UART_LSR_THRE))
			continue;

		write32(UART_THR, c);
		if (c == '\n')
			c = '\r';
		else
			break;
	}

	return 0;
}
