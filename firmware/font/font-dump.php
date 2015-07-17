#!/usr/bin/env php
<?php

define('FONT_FILE', 'lib/SansitaOne.ttf');

define('FONT_HEIGHT', 8);
define('FONT_WIDTH', 8);
define('FONT_OVERSAMPLING', 4);
define('FONT_MAP', '0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ!"#*+-.,\'');

define('WIDTH', FONT_WIDTH*FONT_OVERSAMPLING);
define('HEIGHT', FONT_HEIGHT*FONT_OVERSAMPLING);

function get_char($index, $char)
{
	$bbox = imagettfbbox( HEIGHT, 0, FONT_FILE, $char);

	$x1 = min($bbox[0], $bbox[6]);
	$x2 = max($bbox[2], $bbox[4]);
	$src_width = ($x2 - $x1)*1.2;

	$img = imagecreate($src_width, HEIGHT*1.1);

	$color_bg = imagecolorallocate($img, 0x00, 0x00, 0x00);
	$color_fg = imagecolorallocate($img, 0xFF, 0xFF, 0xFF);
	imagettftext ( $img, HEIGHT, 0 , abs($x1), HEIGHT, $color_fg, FONT_FILE, $char );

	imagepng( $img, $index.'.png' );
}

for($i=0; $i<strlen(FONT_MAP); $i++)
	get_char($i, FONT_MAP[$i]);



