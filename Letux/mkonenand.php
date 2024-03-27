<?php
/*
 * helper script to duplicate all blocks for OneNAND chip
 * with 4k block size (e.g. GTA04, Neo900)
 *
 * OMAP3 TRM tells that in such a case the BootROM reads
 * sectors 0,1,2,3 and skips 4,5,6,7
 *
 * usage: php Letux/mkonenand.php <MLO >MLO.flash
 */

while($block=fread(STDIN, 2048))
	{
	fwrite(STDOUT, $block);
	fwrite(STDOUT, $block);
	}
exit;
?>