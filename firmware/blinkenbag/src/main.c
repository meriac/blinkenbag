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
#include <font.h>
#include "words.h"
#include "cie1931.h"

#define DELAY 200
#define CIE_MAX_INDEX2 (CIE_MAX_INDEX/2)
#define SPI_CS_RGB SPI_CS(LED_PORT,LED_PIN1,6, SPI_CS_MODE_NORMAL )

#define FONT_SPACE (FONT_WIDTH+1)
#define CHARS_PER_DISPLAY (int)((LED_X+FONT_SPACE-1)/FONT_SPACE)

typedef struct {
	uint8_t b, r ,g;
} TRGB;

static double g_time;
static const uint8_t g_latch = 0;
static TRGB g_data[LED_Y][LED_X];

static void update_leds(void)
{
	int x,y;
	TRGB data[LED_COUNT], *dst;

	/* transform array to linear LED strip */
	for(y=0; y<LED_Y; y++)
	{
		dst = &data[y*LED_X];
		if(y&1)
			for(x=(LED_X-1); x>=0; x--)
				*dst++ = g_data[y][x];
		else
			memcpy(dst, &g_data[y], sizeof(g_data[0]));
	}

	/* transmit new values */
	spi_txrx (SPI_CS_RGB, &data, sizeof(data), NULL, 0);

	/* latch data */
	memset(&data, 0, sizeof(data));
	spi_txrx (SPI_CS_RGB, &data, sizeof(data), NULL, 0);
}

static void set_pixel_plasma(int x, int y)
{
	TRGB color, *p;

	if( (x<0) || (x>=LED_X) || (y<0) || (y>LED_Y) )
		return;

	/* update color */
	color.r = (sin( x*0.1+cos(y*0.1+g_time))*CIE_MAX_INDEX2)+CIE_MAX_INDEX2;
	color.g = (cos(-y*0.2-sin(x*0.3-g_time))*CIE_MAX_INDEX2)+CIE_MAX_INDEX2;
	color.b = (cos( x*0.5-cos(y*0.4+g_time))*CIE_MAX_INDEX2)+CIE_MAX_INDEX2;

	/* update pixel */
	p = &g_data[y][x];
	p->r = g_cie[color.r];
	p->g = g_cie[color.g];
	p->b = g_cie[color.b];
}

static void display_words(void)
{
	int i, word;
	const TWordPos *w;

	/* transmit image */
	word = 0;

	while(1)
	{
		/* set background to black */
		memset(g_data, g_cie[0x00], sizeof(g_data));

		/* get next word */
		i = g_sentence[word/DELAY];
		word++;
		if(word>=(WORD_COUNT*DELAY))
			break;

		if(i>=0)
		{
			w = &g_words[i];

			for(i=0; i<w->length; i++)
				set_pixel_plasma(w->x + i, w->y);
		}

		/* send data */
		update_leds();
		/* wait */
		pmu_wait_ms(1);
		g_time+=0.01;
	}
}

static void display_scrolling_draw(char ch, int xpos)
{
	int i, x, y;
	uint8_t data;
	const uint8_t *p;

	/* map to upper case */
	if((ch>='a') && (ch<='z'))
		ch -= 'a'-'A';

	/* search for font entry */
	for(i=0; i<FONT_COUNT; i++)
	{
		p = &g_font[i*FONT_INDEX];
		/* find our character */
		if(*p == (uint8_t)ch)
		{
			/* progress to first data line */
			p++;

			for(y=FONT_HEIGHT-1; y>=0;  y--)
			{
				data = *p++;
				for(x=0; x<FONT_WIDTH; x++)
				{
					if(data & 1)
						set_pixel_plasma( xpos+x, y );
					data >>= 1;
				}
			}
			return;
		}
	}
}

static void display_scrolling(const char* msg)
{
	char c;
	int i, j, pos, x, len, skip;

	len = strlen(msg);

	for(x=0; x<(len*FONT_WIDTH+LED_X); x++)
	{
		/* set background to black */
		memset(g_data, g_cie[0x00], sizeof(g_data));

		/* get position in text */
		i = x / FONT_WIDTH;
		skip = x % FONT_WIDTH;

		/* display one frame */
		for(j=0; j<=CHARS_PER_DISPLAY; j++)
		{
			/* get character from msg */
			pos = i+j;
			c = (pos<len) ? msg[pos] : ' ';

			display_scrolling_draw(c,(j*FONT_SPACE)-skip);
		}

		/* send data */
		update_leds();
		pmu_wait_ms(100);
		g_time+=0.1;
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

	/* setup SPI chipselect pin */
	spi_init ();
	spi_init_pin (SPI_CS_RGB);

	g_time = 0;
	while(1)
	{
		display_scrolling("The quick brown fox jumps over the lazy dog!");
		display_words();
	}
}
