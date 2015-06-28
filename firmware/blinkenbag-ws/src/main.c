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
#include "image.h"

#define IMAGE_OVERSAMPLING 3
#define SIZE_X 8
#define SIZE_Y 8

static double g_time;
static uint8_t g_alpha_channel[SIZE_Y][SIZE_X];
static TRGB g_data[SIZE_Y][SIZE_X];

static void update_leds(void)
{
	int x, y;

	for(y=0; y<SIZE_Y; y++)
		for(x=0; x<SIZE_X; x++)
			rgb_tx(&g_data[y][x]);
	rgb_wait();
}

static void set_pixel_plasma(int x, int y, uint8_t alpha)
{
	TRGB color, *p;

	if( (x<0) || (x>=SIZE_X) || (y<0) || (y>=SIZE_Y) )
		return;

	/* update color */
	color.r = (sin( x*0.1+cos(y*0.1+g_time))*127)+128;
	color.g = (cos(-y*0.2-sin(x*0.3-g_time))*127)+128;
	color.b = (cos( x*0.5-cos(y*0.4+g_time))*127)+128;

	/* update pixel */
	p = &g_data[y][x];
	p->r = (color.r*(int)alpha)/255;
	p->g = (color.g*(int)alpha)/255;
	p->b = (color.b*(int)alpha)/255;
}

static void display_plasma(void)
{
	int x, y;

	/* apply plasma to alpha channel */
	for(y=0; y<SIZE_Y; y++)
		for(x=0; x<SIZE_X; x++)
			set_pixel_plasma(x, y, g_alpha_channel[y][x]);

	/* send data */
	update_leds();
}

void decode(int frame)
{
	int y,x,count;
	const uint8_t *p;
	uint8_t value,t,*out,line[IMAGE_HEIGHT/2];

	for(x=0; x<SIZE_X; x++)
	{
		/* get current line */
		p = &g_img_lines[g_img_lookup_line[frame+(x*IMAGE_OVERSAMPLING)]];

		count = 0;
		out = line;
		while((t = *p++) != 0xF0)
		{
			/* run length decoding case */
			if((t & 0xF0)==0xF0)
			{
				t &= 0xF;
				count += t;
				value = *p++;
				while(t--)
					*out++ = value;
			}
			/* single character */
			else
			{
				*out++ = t;
				count++;
			}
		}

		/* 4 bit unpacking */
		p = line;
		for(y=0; y<(IMAGE_HEIGHT/2); y++)
		{
			t = *p++;

			value = ((t>>0) & 0xF) * IMAGE_VALUE_MULTIPLIER;
			g_alpha_channel[y*2+0][x] = value;

			value = ((t>>4) & 0xF) * IMAGE_VALUE_MULTIPLIER;
			g_alpha_channel[y*2+1][x] = value;
		}
	}
}

int
main (void)
{
	int frame;

	/* Initialize GPIO (sets up clock) */
	GPIOInit ();

	/* Set LED port pin to output */
	GPIOSetDir (LED_PORT, LED_PIN0, 1);
	GPIOSetValue (LED_PORT, LED_PIN0, LED_OFF);

	/* Init Power Management Routines */
	pmu_init ();

	/* initialize WS2812 RGB strip */
	rgb_init();

	memset(&g_alpha_channel, 0xFF, sizeof(g_alpha_channel));
	frame = 0;
	g_time = 0;
	while(1)
	{
		/* draw frame */
//		decode(frame);

		/* handle wrapping */
		frame++;
		if((frame+(SIZE_X*IMAGE_OVERSAMPLING))>=IMAGE_WIDTH)
			frame=0;

		/* transmit current frame */
		display_plasma();

		/* wait and blink */
		GPIOSetValue (LED_PORT, LED_PIN0, LED_ON);
		pmu_wait_ms(1);
		GPIOSetValue (LED_PORT, LED_PIN0, LED_OFF);
		pmu_wait_ms(5);

		g_time+=0.1;
	}
}
