// clang-format off

/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2010
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

#include <nds.h>
#include <nds/arm9/dldi.h>
#include <sys/stat.h>
#include <limits.h>

#include <unistd.h>
#include <fat.h>

#include <string>

/* LHS CHANGE START - load bootlib from disk */
// #include "load_bin.h"
#include <systemfilenames.h>
/* LHS CHANGE END - load bootlib from disk */

#include "nds_loader_arm9.h"
#include "../logger.h"

#ifndef _NO_BOOTSTUB_
#include "bootstub_bin.h"
#include "exceptionstub_bin.h"
#endif

#define LCDC_BANK_C (u16*)0x06840000
#define STORED_FILE_CLUSTER_OFFSET  4
#define WANT_TO_PATCH_DLDI_OFFSET   8   // was 12
#define ARG_START_OFFSET            12  // was 16
#define ARG_SIZE_OFFSET             16  // was 20
#define DLDI_OFFSET_OFFSET 			20
#define HAVE_DSISD_OFFSET           24  // was 28
#define DSIMODE_OFFSET              28  // was 32

typedef signed int addr_t;
typedef unsigned char data_t;

static addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

static void writeAddr (data_t *mem, addr_t offset, addr_t value) {
	((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

static void vramcpy (void* dst, const void* src, int len)
{
	u16* dst16 = (u16*)dst;
	u16* src16 = (u16*)src;
	
	//dmaCopy(src, dst, len);

	for ( ; len > 0; len -= 2) {
		*dst16++ = *src16++;
	}
}

static bool dldiPatchLoader(data_t* binData, u32 binSize)
{
    // dldiOffset is at byte offset 20 in the header
    u32 dldiOff = ((u32*)binData)[DLDI_OFFSET_OFFSET / 4];
    if (dldiOff == 0 || dldiOff >= binSize) {
		// logger().info("DLDI fail: bad offset " + std::to_string(dldiOff));
        return false;
    }

	// logger().info("DldiOff: " + std::to_string(dldiOff));

    DLDI_INTERFACE* target = (DLDI_INTERFACE*)(binData + dldiOff);

    // Bootloader uses 0xBF8DA5EE, not 0xBF8DA5ED, to avoid normal DLDI patchers
    if (target->magicNumber != 0xBF8DA5EE) {
        // logger().info("DLDI fail: wrong magic " + std::to_string(target->magicNumber));
        return false;
    }

    // Check our internal driver fits in the allocated space
    if (io_dldi_data->driverSize > target->allocatedSize) {
		// logger().info("DLDI fail: driver too big, driverSize=" + std::to_string(io_dldi_data->driverSize) + " allocatedSize=" + std::to_string(target->allocatedSize));
        return false;
    }

    // Copy internal driver into the slot and relocate it in place
    u32 copySize = 1 << io_dldi_data->driverSize;
    memcpy(target, io_dldi_data, copySize);

	// Fix all pointers
	void* runtimeAddress = (void*)(0x06000000 + dldiOff);
	dldiRelocate(target, runtimeAddress);

    return true;
}

eRunNdsRetCode runNds(const void* loader, u32 loaderSize, u32 cluster, bool dldiPatchNds, int argc, const char** argv)
{
	char* argStart;
	u16* argData;
	u16 argTempVal = 0;
	int argSize;
	const char* argChar;

	// logger().info("LoaderSize: " + std::to_string(loaderSize));

	irqDisable(IRQ_ALL);

	// Direct CPU access to VRAM bank C
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;

	//Fix VRAM because for some reason some homebrew screws up without it
	//breaks on DSi/3DS mode
	if (!isDSiMode()){
		memset (LCDC_BANK_C, 0x00, 128 * 1024);
	}

	// Load the loader/patcher into the correct address
	vramcpy (LCDC_BANK_C, loader, loaderSize);

	// Set the parameters for the loader
	writeAddr ((data_t*) LCDC_BANK_C, STORED_FILE_CLUSTER_OFFSET, cluster);

	writeAddr ((data_t*) LCDC_BANK_C, DSIMODE_OFFSET, isDSiMode());
	if(argv[0][0]=='s' && argv[0][1]=='d') {
		dldiPatchNds = false;
		writeAddr ((data_t*) LCDC_BANK_C, HAVE_DSISD_OFFSET, 1);
	}

	// WANT_TO_PATCH_DLDI = dldiPatchNds;
	writeAddr ((data_t*) LCDC_BANK_C, WANT_TO_PATCH_DLDI_OFFSET, dldiPatchNds);
	// Give arguments to loader
	argStart = (char*)LCDC_BANK_C + readAddr((data_t*)LCDC_BANK_C, ARG_START_OFFSET);
	argStart = (char*)(((int)argStart + 3) & ~3);	// Align to word
	argData = (u16*)argStart;
	argSize = 0;
	
	for (; argc > 0 && *argv; ++argv, --argc) 
	{
		for (argChar = *argv; *argChar != 0; ++argChar, ++argSize) 
		{
			if (argSize & 1) 
			{
				argTempVal |= (*argChar) << 8;
				*argData = argTempVal;
				++argData;
			} 
			else 
			{
				argTempVal = *argChar;
			}
		}
		if (argSize & 1)
		{
			*argData = argTempVal;
			++argData;
		}
		argTempVal = 0;
		++argSize;
	}
	*argData = argTempVal;

	// logger().info("ArgSize: " + std::to_string(argSize));
	
	writeAddr ((data_t*) LCDC_BANK_C, ARG_START_OFFSET, (addr_t)argStart - (addr_t)LCDC_BANK_C);
	// writeAddr((data_t*) LCDC_BANK_C, ARG_START_OFFSET, (addr_t)argStart - (addr_t)0x06000000);
	writeAddr((data_t*) LCDC_BANK_C, ARG_SIZE_OFFSET, argSize);

	if (dldiPatchNds) {
		// Patch the loader with a DLDI for the card
		if (!dldiPatchLoader((data_t*)LCDC_BANK_C, loaderSize)) {
			// logger().info("Dldi patching failed.");
			return RUN_NDS_PATCH_DLDI_FAILED;
		}
	}

	// logger().info("About to hand off.");

	irqDisable(IRQ_ALL);

	// Give the VRAM to the ARM7
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;

	// Reset into a passme loop
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;
	*((vu32*)0x02FFFFFC) = 0;
	*((vu32*)0x02FFFE04) = (u32)0xE59FF018;
	*((vu32*)0x02FFFE24) = (u32)0x02FFFE04;

	DC_FlushAll();
	IC_InvalidateAll();

	resetARM7(0x06000000);

	// Jump ARM9 directly into the passme loop
	// The bootloader ARM7 will update 0x02FFFE24 to redirect us
	asm volatile("bx %0" : : "r"(0x02FFFE04));
	while(1);

	return RUN_NDS_OK;
}

eRunNdsRetCode runNdsFile(const char* filename, int argc, const char** argv)  {
	struct stat st;
	char filePath[PATH_MAX];
	int pathLen;
	const char* args[1];

	/* LHS CHANGE START - load bootlib from disk */
	u8 *load_bin;
	u32 load_bin_size;

	std::string bootlib = SFN_BOOTLIB;
	FILE* loadBinaryFile = fopen(bootlib.c_str(), "rb");
	if (!loadBinaryFile) {
		return RUN_NDS_LOADER_MISSING;
	}
		
	fseek(loadBinaryFile, 0, SEEK_END);
	load_bin_size = ftell(loadBinaryFile);
	fseek(loadBinaryFile, 0, SEEK_SET);
	load_bin = (u8*)malloc(load_bin_size);
	fread(load_bin, 1, load_bin_size, loadBinaryFile);
	fclose(loadBinaryFile);
	/* LHS CHANGE END - load bootlib from disk */

	if (stat(filename, &st) < 0) {
		return RUN_NDS_STAT_FAILED;
	}

	if (argc <= 0 || !argv) {
		// Construct a command line if we weren't supplied with one
		if (!getcwd (filePath, PATH_MAX)) {
			return RUN_NDS_GETCWD_FAILED;
		}

		pathLen = strlen(filePath);
		strcpy (filePath + pathLen, filename);
		args[0] = filePath;
		argv = args;
	}

	bool havedsiSD = false;
	if(argv[0][0]=='s' && argv[0][1]=='d') {
		havedsiSD = true;
	}
	
	if (!installBootStub(havedsiSD)) {
		return RUN_NDS_STUB_FAILED;
	}

	// logger().info("Argv[0]: " + std::string(argv[0]));

	return runNds(load_bin, load_bin_size, st.st_ino, true, argc, argv);
}

/*
	b	startUp
	
storedFileCluster:
	.word	0x0FFFFFFF		@ default BOOT.NDS
initDisc:
	.word	0x00000001		@ init the disc by default
wantToPatchDLDI:
	.word	0x00000001		@ by default patch the DLDI section of the loaded NDS
@ Used for passing arguments to the loaded app
argStart:
	.word	_end - _start
argSize:
	.word	0x00000000
dldiOffset:
	.word	_dldi_start - _start
dsiSD:
	.word	0
*/

void(*exceptionstub)(void) = (void(*)(void))0x2ffa000;

bool installBootStub(bool havedsiSD) {
#ifndef _NO_BOOTSTUB_
	extern char *fake_heap_end;
	struct __bootstub *bootstub = (struct __bootstub *)fake_heap_end;
	u32 *bootloader = (u32*)(fake_heap_end+bootstub_bin_size);

	memcpy(bootstub,bootstub_bin,bootstub_bin_size);
	memcpy(bootloader,load_bin,load_bin_size);
	bool ret = false;

	bootloader[8] = isDSiMode();
	if( havedsiSD) {
		ret = true;
		bootloader[3] = 0; // don't dldi patch
		bootloader[7] = 1; // use internal dsi SD code
	} else {
		ret = dldiPatchLoader((data_t*)bootloader, load_bin_size,false);
	}
	bootstub->arm9reboot = (VoidFn)(((u32)bootstub->arm9reboot)+fake_heap_end);
	bootstub->arm7reboot = (VoidFn)(((u32)bootstub->arm7reboot)+fake_heap_end);
	bootstub->bootsize = load_bin_size;

	memcpy(exceptionstub,exceptionstub_bin,exceptionstub_bin_size);

	exceptionstub();

	DC_FlushAll();

	return ret;
#else
	return true;
#endif

}