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

#include "../../share/calico/env.h"
#include "../../share/calico/pxi.h"
#include "../../share/fifotool.h"

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

static bool dldiPatchLoader(BootLdrHeader* loader)
{
	const DLDI_INTERFACE* io = io_dldi_data;

	if (!dldiIsValid(io)) {
		// No DLDI patch
		return false;
	}

	void* area = (void*)((u8*)loader + loader->dldiOffset);
	dldiRelocate((DLDI_INTERFACE*)io, area);

	return true;
}

eRunNdsRetCode runNds(const void* loader, u32 loaderSize, u32 cluster, int argc, const char** argv)
{
	char* argStart;
	u16* argData;
	u16 argTempVal = 0;
	int argSize;
	const char* argChar;

	// Direct CPU access to VRAM bank C
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;
	// Load the loader/patcher into the correct address
	memcpy(VRAM_C, loader, loaderSize);

	BootLdrHeader* hdr = (BootLdrHeader*)MM_VRAM_C;

	// Set the parameters for the loader
	hdr->storedFileCluster = cluster;
	hdr->isDsiMode = isDSiMode();

	if(argv[0][0]=='s' && argv[0][1]=='d') {
		hdr->wantToPatchDldi = 0;
		hdr->hasTwlSd = 1;
	} else {
		hdr->hasTwlSd = 0;
	}

	// Give arguments to loader
	argStart = (char*)MM_VRAM_C + hdr->argStart;
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

	hdr->argStart = (uptr)argStart - MM_VRAM_C;
	hdr->argSize = argSize;

	if(hdr->wantToPatchDldi) {
		logger().info("Patching dldi.");
		// Patch the loader with a DLDI for the card
		if (!dldiPatchLoader((BootLdrHeader*)VRAM_C)) {
			return RUN_NDS_PATCH_DLDI_FAILED;
		}
	}

	// Give the VRAM to the ARM7
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;

	// Reset into a passme loop
	*((vu32*)0x02FFFFFC) = 0;
	*(u32*)&g_envAppNdsHeader->title[4] = 0xE59FF018;
	g_envAppNdsHeader->arm9_entrypoint = (u32)&g_envAppNdsHeader->title[4];
	g_envAppNdsHeader->arm7_entrypoint = 0x06000000;
	g_envAppTwlHeader->arm7_mbk_map_settings[0] = mbkMakeMapping(MM_TWLWRAM_MAP, MM_TWLWRAM_MAP+MM_TWLWRAM_BANK_SZ, MbkMapSize_256K);
	g_envExtraInfo->pm_chainload_flag = 1;

	// Provide all needed hardware to the ARM7
    // VRAM to ARM7 execution mode
    vramSetPrimaryBanks(VRAM_A_LCD, VRAM_B_LCD, VRAM_C_ARM7, VRAM_D_ARM7);
    // Slot 1 and Slot 2
    sysSetBusOwners(BUS_OWNER_ARM7, BUS_OWNER_ARM7);

    // Flush cache
    CP15_CleanAndFlushDCache();
    CP15_FlushICache();

	fifoSendValue32(FIFO_USER_01, MENU_MSG_ARM7_REBOOT_NDS);

	swiDelay(40);

    // // Disable all IRQs
    // irqDisable(IRQ_ALL);
    // REG_IME = IME_DISABLE;
    // REG_IE = 0;
    // REG_IF = ~0;

	((void(*)(void))g_envAppNdsHeader->arm9_entrypoint)();

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

	return runNds(load_bin, load_bin_size, st.st_ino, argc, argv);
}

bool installBootStub(bool havedsiSD) {
#ifndef _NO_BOOTSTUB_
	void* bootstub = g_envNdsBootstub;
	BootLdrHeader *bootloader = (BootLdrHeader*)((u8*)bootstub+bootstub_bin_size);

	armCopyMem32(bootstub,bootstub_bin,bootstub_bin_size);
	armCopyMem32(bootloader,load_bin,load_bin_size);
	bool ret = false;

	bootloader->isDsiMode = isDSiMode();
	if( havedsiSD) {
		ret = true;
		bootloader->wantToPatchDldi = 0;
		bootloader->hasTwlSd = 1;
	} else {
		ret = dldiPatchLoader(bootloader);
	}

	g_envNdsBootstub->arm9_entrypoint = (void*)((u32)bootstub+(u32)g_envNdsBootstub->arm9_entrypoint);
	g_envNdsBootstub->arm7_entrypoint = (void*)((u32)bootstub+(u32)g_envNdsBootstub->arm7_entrypoint);
	*(u32*)(g_envNdsBootstub+1) = load_bin_size;

	DC_FlushAll();

	return ret;
#else
	return true;
#endif

}