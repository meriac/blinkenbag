#!/usr/bin/env php
<?php

define('FONT_FILE', 'lib/SansitaOne.ttf');

define('FONT_HEIGHT', 8);
define('FONT_WIDTH', 8);
define('IMAGE_MULTIPLIER', 17);
define('IMAGE_OVERSAMPLING', 3);
define('FONT_MAP', '0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ!"#*+-.,\'');

define('WIDTH', FONT_WIDTH*IMAGE_OVERSAMPLING);
define('HEIGHT', FONT_HEIGHT*IMAGE_OVERSAMPLING);

function compress_img($index, $img)
{
	global $table_offset;

	$sx = imagesx($img);
	$sy = imagesy($img);

	for($x=0; $x<$sx; $x++)
	{
		$line = array();

		for($y=0; $y<$sy; $y++)
		{
			$rgb = imagecolorat ($img, $x, $y);

			$r = ($rgb >> 16) & 0xFF;
			$g = ($rgb >>  8) & 0xFF;
			$b = ($rgb >>  0) & 0xFF;
			$gray = intval((($r+$g+$b)/3)/IMAGE_MULTIPLIER);

			$line[] = $gray;
		}

		/* convert to 4 bit packed */
		$compressed = array();
		$first = true;
		for($y=0; $y<($sy/2); $y++)
			$compressed[] = $line[($y*2)+0] | ($line[($y*2)+1]<<4);

		printf("\t/*0x%04X*/ ",$table_offset);
		foreach($compressed as $value)
			printf("0x%02X,",$value);
		echo "\n";

		$table_offset += count($compressed);
	}
}

function get_char($index, $char)
{
	$bbox = imagettfbbox( HEIGHT, 0, FONT_FILE, $char);

	$x1 = min($bbox[0], $bbox[6]);
	$x2 = max($bbox[2], $bbox[4]);
	$src_width = ($x2 - $x1)*1.2;

	$img = imagecreate($src_width, HEIGHT*1.1);

	$color_bg = imagecolorallocate($img, 0x00, 0x00, 0x00);
	$color_fg = imagecolorallocate($img, 0xFF, 0xFF, 0xFF);
	imagettftext ( $img, HEIGHT, 0 , abs($x1), HEIGHT*0.93, $color_fg, FONT_FILE, $char );

	$frame = imagecreatetruecolor($src_width, FONT_HEIGHT);
	imagecopyresampled($frame, $img, 0, 0, 0, 0, $src_width, FONT_HEIGHT, $src_width, imagesy($img));

	return $frame;
}


echo "#ifndef __IMAGE_H__\n";
echo "#define __IMAGE_H__\n";

printf("\n/* auto generated with %s\n   using the font '%s' */\n", basename(__FILE__), FONT_FILE);

echo "\nconst uint8_t g_img_lines[] = {\n";

$list_offset = array();
$table_offset = 0;

for($i=0; $i<strlen(FONT_MAP); $i++)
{
	/* remember list offest */
	$list_offset[] = $table_offset;

	/* compress character */
	$char = FONT_MAP[$i];
	$img = get_char($i, $char);
	compress_img($i, $img);
}

printf("\t/*0x%04X*/ 0xF0};\n", $table_offset);

echo "\nconst uint16_t g_img_lookup_char[] = {";

foreach($list_offset as $index => $offset)
{
	if(($index % 8) == 0)
		echo "\n\t";
	printf("0x%04X,", $offset);
}
printf("\n\t0x%04X};\n", $table_offset);

echo "\nconst char g_img_font_map[] = \"".addcslashes(FONT_MAP,'\\"')."\";\n";

echo "\n";
printf("#define IMAGE_FONT_CHARS %u\n",strlen(FONT_MAP));
printf("#define IMAGE_SIZE %u\n",$table_offset);
printf("#define IMAGE_VALUE_MULTIPLIER %u\n", IMAGE_MULTIPLIER);
printf("#define IMAGE_OVERSAMPLING_X %u\n", IMAGE_OVERSAMPLING);
printf("#define IMAGE_HEIGHT %u\n", FONT_HEIGHT);

echo "\n#endif/*__IMAGE_H__*/\n";
