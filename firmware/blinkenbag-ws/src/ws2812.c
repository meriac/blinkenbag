/***************************************************************
 *
 * blinkenbag.org - WS2812 routines for LPC13xx
 *
 * Copyright 2015 Milosch Meriac <milosch@meriac.com>
 *
 ***************************************************************

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 */
#include <openbeacon.h>
#include "cie1931.h"
#include "ws2812.h"

#define BIT_REVERSE(x) ((unsigned char)(__RBIT(x)>>24))

#define SPI_BITS_H 8
#define SPI_BITS_L 4
#define SPI_BITS 13
#define RGB_BITS 24

int rgb_tx (uint32_t rgb)
{
	int bits, colour, data;

	/* colour correction & channel shuffle */
	colour =  g_cie[(rgb >>  8) & 0xFF];
	colour <<= 8;
	colour |= g_cie[(rgb >> 16) & 0xFF];
	colour <<= 8;
	colour |= g_cie[rgb & 0xFF];

	/* transmit all 24 bits */
	bits = RGB_BITS;
	while(bits>0)
	{
		/* while TX not full */
		if(LPC_SSP->SR & 2)
		{
			/* bit-dependent symbol */
			data = (colour & (1UL<<(RGB_BITS-1))) ?
				((1<<SPI_BITS_H)-1)|(SPI_BITS-SPI_BITS_H):
				((1<<SPI_BITS_L)-1)|(SPI_BITS-SPI_BITS_L);
			colour <<= 1;
			bits--;

			/* transmit symbol */
			LPC_SSP->DR = data;
		}

		if(LPC_SSP->SR & 4)
			data = LPC_SSP->DR;
	}

	/* Wait for TX transaction to finish */
	while ((LPC_SSP->SR & 1) == 0);

	return 0;
}

void
rgb_init (void)
{
	/* reset SSP peripheral */
	LPC_SYSCON->PRESETCTRL = 0x01;

	/* Enable SSP clock */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 11);

	/* Enable MOSI */
	LPC_IOCON->PIO0_9 = 0x01;
	/* route to PIO0_10 */
	LPC_IOCON->SCKLOC = 0x00;
	/* SCK */
	LPC_IOCON->JTAG_TCK_PIO0_10 = 0x02;

	/* Set SSP PCLK to 72MHz using DIV=1 */
	LPC_SYSCON->SSPCLKDIV = 1;

	/* 13 bit, SPI, SCR=0 */
	LPC_SSP->CR0  = 0x000C;
	LPC_SSP->CR1  = 0x0002;
	/* divide clock further down to 10.29MHz (DIV=7) */
	LPC_SSP->CPSR = 7;
}

void
rgb_close (void)
{
	/* Disable SSP clock */
	LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 11);

	/* Disable SSP PCLK */
	LPC_SYSCON->SSPCLKDIV = 0x00;

	/* MOSI */
	LPC_IOCON->PIO0_9 = 0x00;
	GPIOSetDir (0, 9, 1);  /* OUT */
	GPIOSetValue (0, 9, 0);

	/* SCK */
	LPC_IOCON->JTAG_TCK_PIO0_10 = 0x01;
	GPIOSetDir (0, 10, 1); /* OUT */
	GPIOSetValue (0, 10, 0);
}
