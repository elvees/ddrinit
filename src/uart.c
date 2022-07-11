// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2020 RnD Center "ELVEES", JSC

#include <common.h>
#include <regs.h>
#include <plat/plat.h>
#include <uart.h>

#define UART_LCR_DEFAULT 3 /* 8 bit, no parity, 1 stop bit. */

int uart_cfg(void)
{
	int ret;
	uint16_t divisor;

	ret = platform_uart_cfg();
	if (ret)
		return ret;

	/* Set baudrate */
	divisor = DIV_ROUND_CLOSEST(CONFIG_UART_BASE_FREQ, CONFIG_UART_BAUDRATE * 16);
	write32(UART_LCR, UART_LCR_DEFAULT | UART_LCR_DLAB);
	write32(UART_DLH, divisor >> 8);
	write32(UART_DLL, divisor);
	write32(UART_LCR, UART_LCR_DEFAULT);

	/* Make sure last LCR write wasn't ignored */
	while (read32(UART_LCR) != UART_LCR_DEFAULT) {
		write32(UART_FCR, 0);
		write32(UART_FCR, UART_FCR_FIFOE);
		write32(UART_FCR, UART_FCR_FIFOE | UART_FCR_XFIFOR | UART_FCR_RFIFOR);
		read32(UART_THR);
		write32(UART_LCR, UART_LCR_DEFAULT);
	}

	write32(UART_IER, 0);
	write32(UART_MCR, UART_MCR_DTR | UART_MCR_RTS);

	return 0;
}

int putchar(int c)
{
	int ret;
	uint32_t val = 0;

	while (1) {
		ret = read32_poll_timeout(val, val & UART_LSR_THRE, USEC, 100 * USEC, UART_LSR);
		if (ret)
			return ret;

		write32(UART_THR, c);
		if (c == '\n')
			c = '\r';
		else
			break;
	}

	return 0;
}
