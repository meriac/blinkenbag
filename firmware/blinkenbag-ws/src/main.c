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

#define SIZE_X 32
#define SIZE_Y 8
#define SCALE 128

static float g_time;
static uint8_t g_alpha_channel[SIZE_X][SIZE_Y];
static TRGB g_data[SIZE_X][SIZE_Y];

static const char g_text[] = "The quick brown fox jumps over the lazy dog";
int g_text_pos, g_text_pos_sub;

static void update_leds(void)
{
	int x, y;

	for(x=0; x<SIZE_X; x++)
		for(y=0; y<SIZE_Y; y++)
			rgb_tx(&g_data[x][(x&1)? (SIZE_Y-1)-y : y]);
	rgb_wait();
}

static void set_pixel_plasma(int x, int y, uint8_t alpha)
{
	TRGB color, *p;

	if( (x<0) || (x>=SIZE_X) || (y<0) || (y>=SIZE_Y) )
		return;

	/* update color */
	color.r = (sinf( x*0.1+cosf(y*0.1+g_time))*(SCALE-1))+SCALE;
	color.g = (cosf(-y*0.2-sinf(x*0.3-g_time))*(SCALE-1))+SCALE;
	color.b = (cosf( x*0.5-cosf(y*0.4+g_time))*(SCALE-1))+SCALE;

	/* update pixel */
	p = &g_data[x][y];
	p->r = (color.r*(int)alpha)/255;
	p->g = (color.g*(int)alpha)/255;
	p->b = (color.b*(int)alpha)/255;
}

static void display_plasma(void)
{
	int x, y;

	/* apply plasma to alpha channel */
	for(x=0; x<SIZE_X; x++)
		for(y=0; y<SIZE_Y; y++)
			set_pixel_plasma(x, y, g_alpha_channel[x][y]);

	/* send data */
	update_leds();
}

int find_char(char target)
{
	int res;
	char c;
	const char *p = g_img_font_map;

	/* convert to upper case */
	if((target>='a') && (target<='z'))
		target -= 'a'-'A';

	/* find character */
	res = 0;
	while( (c = *p++)!=0 )
		if(c==target)
			return res;
		else
			res++;

	/* return negative count if target can't be found in font map */
	return -1;
}

void decode(void)
{
	int x, y;
	const uint8_t *v;
	uint8_t data;

	for(x=0; x<SIZE_X; x++)
	{
		v = &g_img_lines[(g_text_pos*(SIZE_Y/2)) + (x*IMAGE_OVERSAMPLING_X*(SIZE_Y/2))];

		for(y=0; y<(SIZE_Y/2); y++)
		{
			data = *v++;
			g_alpha_channel[x][y*2+0] = (data & 0xF) * (IMAGE_VALUE_MULTIPLIER/2);
			g_alpha_channel[x][y*2+1] = (data >> 4) * (IMAGE_VALUE_MULTIPLIER/2);
		}
	}
}

int
main (void)
{
	/* Initialize GPIO (sets up clock) */
	GPIOInit ();

	/* Set LED port pin to output */
	GPIOSetDir (LED_PORT, LED_PIN0, 1);
	GPIOSetValue (LED_PORT, LED_PIN0, LED_OFF);

	/* Init Power Management Routines */
	pmu_init ();

	/* initialize WS2812 RGB strip */
	rgb_init();

	g_time = 0;
	g_text_pos = g_text_pos_sub = 0;
	while(1)
	{
		/* draw frame */
		decode();

		g_text_pos++;
		if((g_text_pos+(SIZE_X*IMAGE_OVERSAMPLING_X))>=(int)(sizeof(g_img_lines)/(SIZE_Y/2)))
			g_text_pos = 0;

		/* transmit current frame */
		display_plasma();

		/* wait and blink */
		GPIOSetValue (LED_PORT, LED_PIN0, LED_ON);
		pmu_wait_ms(1);
		GPIOSetValue (LED_PORT, LED_PIN0, LED_OFF);

		g_time+=0.1;
	}
}
