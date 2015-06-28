#!/bin/php
<?php

define('BITMAP_SIZE', 8);
define('OVERSAMPLING', 3);
define('FONT_FILE', 'lib/SansitaOne.ttf');

$text = "Far out in the uncharted backwaters of the unfashionable"
	." end of the western spiral arm of the Galaxy lies a smal"
	."l unregarded yellow sun. Orbiting this at a distance of "
	."roughly ninety-two million miles is an utterly insignifi"
	."cant little blue-green planet whose ape-descended life f"
	."orms are so amazingly primitive that they still think di"
	."gital watches are a pretty neat idea.";

$text = strtoupper($text);
$size = imagettfbbox( (BITMAP_SIZE+1) * OVERSAMPLING, 0, FONT_FILE, $text);
$src_width = abs($size[4] - $size[0]);
$src_height = abs($size[5] - $size[1]);

$src = imagecreatetruecolor($src_width, $src_height);
$color = imagecolorallocate($src, 0xFF, 0xFF, 0xFF);
imagettftext ( $src, (BITMAP_SIZE+1) * OVERSAMPLING, 0 , 0, $src_height - $size[1], $color, FONT_FILE, $text );

$frame = imagecreatetruecolor($src_width, BITMAP_SIZE);
imagecopyresampled($frame, $src, 0, 0, 0, 0, $src_width, BITMAP_SIZE, $src_width, $src_height);

imagepng($frame, "scroll-text.png");
