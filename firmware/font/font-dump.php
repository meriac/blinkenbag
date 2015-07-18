#!/usr/bin/env php
<?php

define('FONT_FILE', 'lib/SansitaOne.ttf');

define('FONT_HEIGHT', 8);
define('FONT_WIDTH', 8);
define('IMAGE_MULTIPLIER', 3);
define('FONT_MAP', '0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ!"#*+-.,\'');

define('WIDTH', FONT_WIDTH*IMAGE_MULTIPLIER);
define('HEIGHT', FONT_HEIGHT*IMAGE_MULTIPLIER);

function compress_img($index, $img)
{
	global $dist, $lookup, $table_offset;

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
		$debug = array();
		for($y=0; $y<($sy/2); $y++)
		{
			$data = $line[($y*2)+0] | ($line[($y*2)+1]<<4);
			$debug[] = $data;

			if($first)
			{
				$first = false;
				$prev = $data;
				$count = 1;
			}
			else
			{
				if($data==$prev)
				{
					$count++;
					if($count>=15)
					{
						$compressed[] = 0xF0 | $count;
						$compressed[] = $prev;
						$count = 0;
					}
				}
				else
				{
					if($count==1)
						$compressed[] = $prev;
					else
					{
						if($count>0)
						{
							$compressed[] = 0xF0 | $count;
							$compressed[] = $prev;
						}
						$count = 1;
					}
					$prev = $data;
				}
			}
		}
		/* dump remaining data */
		if($count)
		{
			$compressed[] = 0xF0 | $count;
			$compressed[] = $data;
		}
		$compressed[] = 0xF0;

		$s = implode(',',$compressed);
		if(isset($dist[$s]))
			$lookup[] = $dist[$s];
		else
		{
			printf("\t/*0x%04X*/ ",$table_offset);
			foreach($compressed as $value)
				printf("0x%02X,",$value);
			echo "\n";

			$lookup[] = $table_offset;
			$dist[$s] = $table_offset;
			$table_offset += count($compressed);
		}
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

printf("\n/* auto generated with %s\n   using image %s */\n", __FILE__, IMAGE_NAME);

echo "\nconst uint8_t g_img_lines[] = {\n";

$dist = array();
$lookup = array();
$list_offset = array();
$table_offset = 0;

for($i=0; $i<strlen(FONT_MAP); $i++)
{
	/* remember list offest */
	$list_offset[] = count($lookup);

	/* compress character */
	$char = FONT_MAP[$i];
	$img = get_char($i, $char);
	compress_img($i, $img);
}

printf("\t/*0x%04X*/ 0xF0};\n", $table_offset);

echo "\nconst uint16_t g_img_lookup_line[] = {";

foreach($lookup as $index => $offset)
{
	if(($index % 8) == 0)
		echo "\n\t";
	printf("0x%04X,", $offset);
}

echo "\n\t0xFFFF};\n";

echo "\nconst uint16_t g_img_lookup_char[] = {";

foreach($list_offset as $index => $offset)
{
	if(($index % 8) == 0)
		echo "\n\t";
	printf("0x%04X,", $offset);
}

echo "\n\t0xFFFF};\n";

echo "\nconst char g_img_font[] = \"".addcslashes(FONT_MAP,'\\\'')."\"\n";

echo "\n";
printf("#define IMAGE_FONT_CHARS %u\n",strlen(FONT_MAP));
printf("#define IMAGE_SIZE   %u\n",$table_offset + count($lookup)*2);
printf("#define IMAGE_VALUE_MULTIPLIER %u\n", IMAGE_MULTIPLIER);

echo "\n#endif/*__IMAGE_H__*/\n";
