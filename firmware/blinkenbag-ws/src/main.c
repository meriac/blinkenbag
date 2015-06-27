/***************************************************************
 *
 * OpenBeacon.org - RGB Strip control
 *
 * Copyright 2014 Milosch Meriac <milosch@meriac.com>
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
#include "ws2812.h"

int
main (void)
{
	int x,y;

	/* Initialize GPIO (sets up clock) */
	GPIOInit ();

	/* Set LED port pin to output */
	GPIOSetDir (LED_PORT, LED_PIN0, 1);
	GPIOSetValue (LED_PORT, LED_PIN0, LED_OFF);

	/* Init Power Management Routines */
	pmu_init ();

	/* initialize WS2812 RGB strip */
	rgb_init();

	while(1)
	{
		for(x=0; x<144; x++)
		{
			for(y=0; y<144; y++)
				rgb_tx(y==x ? 0x800000 : 0);

			/* wait and blink */
			pmu_wait_ms(5);
			GPIOSetValue (LED_PORT, LED_PIN0, LED_ON);
			pmu_wait_ms(5);
			GPIOSetValue (LED_PORT, LED_PIN0, LED_OFF);
		}
	}
}
