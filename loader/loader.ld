/* Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
   GameCube Port Copyright 2016 FIX94
   This code is licensed to you under the terms of the GNU GPL, version 2;
   see file LICENSE or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt */

OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

SECTIONS {
	. = 0x80001800;

	.start : { start.o(*) }
	.text : { *(.text) }
	.rodata : { *(.rodata .rodata.*)}
	.data : { *(.data) }

	__bss_start = .;
	.bss : { *(.bss) }
	__bss_end = .;

	. = ALIGN(0x40);
	.stack : {
		. += 0x8000;
		_stack_top = .;
	}
}
