// Copyright 2007,2008  Segher Boessenkool  <segher@kernel.crashing.org>
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

// File altered to fix checksum for Wind Waker instead of Twilight Princess

#include <stdio.h>

#include "tools.h"

static u8 buf[0x4000];

static void slot_checksum(u8 *x)
{
	u32 i;
	u32 sum, nsum;

	sum = 0;
	for (i = 0; i < 0x0768; i++)
		sum += x[i];
	nsum = -(sum + 0x0768);

	wbe32(x + 0x0768, sum);
	wbe32(x + 0x076c, nsum);
}

static void save_checksum(u8 *x)
{
	u32 i;
	u16 sum, nsum;

	slot_checksum(x + 8);
	slot_checksum(x + 8 + 0x0770);
	slot_checksum(x + 8 + 2*0x0770);

	sum = 0;
	for (i = 0; i < 0x0ffe; i++)
		sum += be16(x + 2*i);
	nsum = -(sum + 0x0ffe);

	wbe16(x + 0x1ffc, sum);
	wbe16(x + 0x1ffe, nsum);
}

int main(int argc, char **argv)
{
	FILE *fp;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <zeldaWW.dat>\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "rb+");
	if (!fp)
		fatal("open %s", argv[1]);
	if (fread(buf, 0x4000, 1, fp) != 1)
		fatal("read %s", argv[1]);

	save_checksum(buf);
	save_checksum(buf + 0x2000);

	if (fseek(fp, 0, SEEK_SET))
		fatal("seek");
	if (fwrite(buf, 0x4000, 1, fp) != 1)
		fatal("write %s", argv[1]);
	fclose(fp);

	return 0;
}
