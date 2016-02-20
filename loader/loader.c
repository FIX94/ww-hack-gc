// Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
// GameCube Port Copyright 2016 FIX94
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file LICENSE or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef struct _dolheader {
	u32 text_pos[7];
	u32 data_pos[11];
	u32 text_start[7];
	u32 data_start[11];
	u32 text_size[7];
	u32 data_size[11];
	u32 bss_start;
	u32 bss_size;
	u32 entry_point;
} dolheader;

static u8 dolData[0x200] __attribute__ ((aligned(32)));

#define vu16 volatile unsigned short
#define TO_ARAM 0
#define TO_MRAM 1
void ar_dma(u32 type, u32 mram, u32 aram, u32 len)
{
	*(vu16*)0xcc005020 = (mram>>16);
	*(vu16*)0xcc005022 = (mram&0xFFFF);
	*(vu16*)0xcc005024 = (aram>>16);
	*(vu16*)0xcc005026 = (aram&0xFFFF);
	*(vu16*)0xcc005028 = (type<<15)|(len>>16);
	*(vu16*)0xcc00502A = (len&0xFFFF);
	while(*(vu16*)0xcc00500A & 0x200) ;
}

static void sync_cache(void *p, u32 n)
{
	u32 start, end;

	start = (u32)p & ~31;
	end = ((u32)p + n + 31) & ~31;
	n = (end - start) >> 5;

	while (n--) {
		asm("dcbst 0,%0 ; icbi 0,%0" : : "b"(p));
		p += 32;
	}
	asm("sync ; isync");
}

static void sync_before_read(void *p, u32 n)
{
	u32 start, end;

	start = (u32)p & ~31;
	end = ((u32)p + n + 31) & ~31;
	n = (end - start) >> 5;

	while (n--) {
		asm("dcbf 0,%0" : : "b"(p));
		p += 32;
	}
	asm("sync");
}

void *_memset(void *ptr, int c, int size) {
	char* ptr2 = ptr;
	while(size--) *ptr2++ = (char)c;
	return ptr;
}

void *_memcpy(void *ptr, const void *src, int size) {
	char* ptr2 = ptr;
	const char* src2 = src;
	while(size--) *ptr2++ = *src2++;
	return ptr;
}

void readAlign(u8* bufPos, u32 length, u32 unalignedOffset)
{
	u32 alignedOffset = unalignedOffset & (~0x1FF);
	u32 offsetDiff = unalignedOffset - alignedOffset;

	if(offsetDiff > 0)
	{
		sync_before_read(dolData, 0x200);
		ar_dma(TO_MRAM, (u32)dolData, alignedOffset, 0x200);
		u32 write = ((length > 0x200) ? 0x200 : length) - offsetDiff;
		_memcpy(bufPos, dolData + offsetDiff, write);
		sync_cache(bufPos, write);
		alignedOffset += 0x200;
		bufPos += write;
		length -= write;
	}

	while(length > 0)
	{
		sync_before_read(dolData, 0x200);
		ar_dma(TO_MRAM, (u32)dolData, alignedOffset, 0x200);
		u32 write = ((length > 0x200) ? 0x200 : length);
		_memcpy(bufPos, dolData, write);
		sync_cache(bufPos, write);
		alignedOffset += 0x200;
		bufPos += write;
		length -= write;
	}
}

void __attribute__ ((noreturn)) _main()
{
	int i;
	dolheader dolHdr;
	sync_before_read(dolData, 0x200);
	ar_dma(TO_MRAM, (u32)dolData, 0, 0x200);
	_memcpy(&dolHdr, dolData, sizeof(dolheader));
	//write down boot.dol in memory
	for (i = 0; i < 7; i++)
	{
		if ((!dolHdr.text_size[i]) || (dolHdr.text_start[i] < 0x100))
			continue;
		readAlign((u8*)dolHdr.text_start[i], dolHdr.text_size[i], dolHdr.text_pos[i]);
	}
	for (i = 0; i < 11; i++)
	{
		if ((!dolHdr.data_size[i]) || (dolHdr.data_start[i] < 0x100))
			continue;
		readAlign((u8*)dolHdr.data_start[i], dolHdr.data_size[i], dolHdr.data_pos[i]);
	}
	//lets jump
	__asm__ volatile(
		"mtlr %0\n"
		"blr\n"
	 : : "r"(dolHdr.entry_point));
	 __builtin_unreachable();
}
