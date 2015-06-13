#!/usr/bin/env php
<?php

/***************************************************************
 *
 * blinkenbag.org - font header file generation
 *
 * Copyright 2015 Milosch Meriac <milosch@meriac.com>
 *
/***************************************************************

/*
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

define('FONT_FILE', 'font.png');
define('FONT_WIDTH', 5);
define('FONT_HEIGHT', 5);
define('FONT_MAP', '0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]{}!"#()*+-/.\'');

function read_character($img, $posx)
{
	$res = array();

	for($y=0; $y<FONT_HEIGHT; $y++)
	{
		$byte = 0;
		for($x=FONT_WIDTH-1; $x>=0; $x--)
			$byte = ($byte<<1) | (imagecolorat($img, $posx+$x, $y) ? 1:0);

		$res[] = $byte;
	}

	return $res;
}

/* read font image */
if(!($png = @imagecreatefrompng(FONT_FILE)))
	exit("Failed to open ".FONT_FILE.PHP_EOL);

/* cakculate character count from image */
$count = floor(imagesx($png) / FONT_WIDTH);
/* check image size */
$map_len = strlen(FONT_MAP);
if($count != $map_len)
	exit("font map length ($map_len) does not match image font count ($count)".PHP_EOL);

/* dump c-header file */
echo '#ifndef __FONT_H__'.PHP_EOL;
echo '#define __FONT_H__'.PHP_EOL.PHP_EOL;

echo '#define FONT_WIDTH '.FONT_WIDTH.PHP_EOL;
echo '#define FONT_HEIGHT '.FONT_HEIGHT.PHP_EOL;
echo '#define FONT_INDEX '.(FONT_WIDTH+1).PHP_EOL;
echo '#define FONT_COUNT '.$map_len.PHP_EOL;

/* dump all font characters */
echo PHP_EOL.'const uint8_t g_font[FONT_COUNT*FONT_INDEX] = {'.PHP_EOL;
for($i=0; $i<$count; $i++)
{
	printf("\t%s'%s'", $i?',':' ', addcslashes(FONT_MAP[$i],'\\\''));
	foreach(read_character($png, $i*FONT_WIDTH) as $line)
		printf(",0x%02X", $line);
	echo PHP_EOL;
}
echo "};".PHP_EOL;

echo PHP_EOL.'#endif/*__FONT_H__*/'.PHP_EOL;
