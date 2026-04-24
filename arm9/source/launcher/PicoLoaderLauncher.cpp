/*
    Copyright (C) 2024 lifehackerhansol
    Additional modifications Copyright (C) 2025 coderkei

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <malloc.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <cstring>
#include <nds/arm9/dldi.h>
#include <nds/arm9/video.h>
#include <nds/bios.h>
#include <nds/dma.h>
#include "../blockds/cp15.h"
#include "../blockds/dma.h"
#include "../blockds/ndma.h"
#include <nds/fifocommon.h>
#include <nds/interrupts.h>
#include <nds/memory.h>
#include <nds/ndstypes.h>

#include "PicoLoaderLauncher.h"
#include "tonccpy.h"
#include "../cheat.h"
#include "../flags.h"
#include "../mainlist.h"
#include "../systemfilenames.h"
#include "../language.h"
#include "../ui/msgbox.h"
#include "../ui/progresswnd.h"

constexpr std::align_val_t cache_align { 32 };

typedef void (*pico_loader_9_func_t)(void);

TaskWorker* PicoLoaderLauncher::task() {
    return this;
}

void PicoLoaderLauncher::copyToVram(const char* loaderPath, void* destination) {
    u32 loaderSize = 0;
    u8* loaderBuffer;

    FILE* f = fopen(loaderPath, "rb");
    fseek(f, 0, SEEK_END);
    loaderSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    // while we aren't using DMA now, it's good to keep in mind that we totally could in the future
    loaderBuffer = (u8*)memalign(32, loaderSize);
    fread(loaderBuffer, 1, loaderSize, f);
    CP15_CleanAndFlushDCacheRange(loaderBuffer, loaderSize);
    memcpy(destination, loaderBuffer, loaderSize);
    fclose(f);
    free(loaderBuffer);
}

bool PicoLoaderLauncher::prepareCheats(void) {
    u32* cheatTablePtr;
    std::vector<cCheatDatItem> cheatEntries;
    // a pload_cheats_t table always has two u32s as the header
    u32 cheatTableSize = 8;
    cCheat cheat;

    if (!cheat.parse(_romPath)) goto cheat_failed;

    cheatEntries = cheat.getEnabledCheats();
    if (cheatEntries.empty()) goto cheat_failed;

    // calculate size of pload_cheats_t to allocate
    for (u32 i = 0; i < cheatEntries.size(); i++) {
        // Each pload_cheats_t table has a pload_cheat_t entry for each cheat entry
        // which has a u32 for length
        cheatTableSize += 4;
        // then the data, multiplied by 4 since each opcode is a u32
        cheatTableSize += ((cheatEntries[i]._cheat.size()) << 2);
    }

    // construct the cheat table
    mCheats = (pload_cheats_t*)memalign(4, cheatTableSize);
    mCheats->length = cheatTableSize;
    mCheats->numberOfCheats = cheatEntries.size();
    cheatTablePtr = (u32*)mCheats;

    // load cheat codes to table
    // skip the first two u32, which is the cheat table's header
    cheatTablePtr += 2;

    for (u32 i = 0; i < cheatEntries.size(); i++) {
        // pload_cheat_t->length
        *cheatTablePtr = cheatEntries[i]._cheat.size() << 2;

        cheatTablePtr++;
        // pload_cheat_opcode_t, arbitrary size
        // TODO bounds check
        for (u32 j = 0; j < cheatEntries[i]._cheat.size(); j++) {
            *cheatTablePtr = cheatEntries[i]._cheat[j];
            cheatTablePtr++;
        }
    }

    return true;

cheat_failed:
    return false;
}

bool PicoLoaderLauncher::setParameters(void) {
    pload_header7_t* picoLoaderHeader7 = (pload_header7_t*)0x06840000;
    pload_params_t* picoLoaderParams = (pload_params_t*)memalign(32, sizeof(pload_params_t));

    // Set Pico Loader's DLDI pointer to the current driver.
    // Pico Loader should relocate it to a safe place.
    picoLoaderHeader7->dldiDriver = io_dldi_data;
    // Set Pico Loader's boot drive to DLDI.
    // Pico Loader does not support SD loading for retail games, and for homebrew loading we use the
    // reference bootlib instead. We also don't have proper support for AGB semihosting at this
    // time.
    picoLoaderHeader7->bootDrive = PLOAD_BOOT_DRIVE_DLDI;

    // Start adding Pico Loader parameters
    memset(picoLoaderParams, 0, sizeof(pload_params_t));
    strncpy(picoLoaderParams->romPath, _romPath.c_str(),
            _romPath.size() < 256 ? _romPath.size() : 256 - 1);
    strncpy(picoLoaderParams->savePath, _savePath.c_str(),
            _savePath.size() < 256 ? _savePath.size() : 256 - 1);
    memcpy(&(picoLoaderHeader7->loadParams), picoLoaderParams, sizeof(pload_params_t));
    free(picoLoaderParams);

    std::strcpy(picoLoaderHeader7->v2.launcherPath, "/_nds/akmenunext/launcher.nds");

    if (!_hb && _flags & PATCH_CHEATS) {
        prepareCheats();
        picoLoaderHeader7->v3.cheats = mCheats;
    }

    return true;
}

s16 PicoLoaderLauncher::process(s16 iter) {
    static const char picoLoaderPath7[] = "fat:/_pico/picoLoader7.bin";
    static const char picoLoaderPath9[] = "fat:/_pico/picoLoader9.bin";

    if (access(picoLoaderPath7, F_OK) != 0) {
        showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), picoLoaderPath7));
        return -1;
    }
    if (access(picoLoaderPath9, F_OK) != 0) {
        showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), picoLoaderPath9));
        return -1;
    }

    // POINT OF NO RETURN = start overwriting VRAM
    vramSetPrimaryBanks(VRAM_A_LCD, VRAM_B_LCD, VRAM_C_LCD, VRAM_D_LCD);

    // Load ARM7
    copyToVram(picoLoaderPath9, VRAM_A);
    copyToVram(picoLoaderPath7, VRAM_C);

    // Disable DMA and NDMA
    for (int i = 0; i <= 3; i++) {
        dmaStopSafe(i);
    }

    if (isDSiMode()) {
        NDMA0_CR = 0;
        NDMA1_CR = 0;
        NDMA2_CR = 0;
        NDMA3_CR = 0;
    }

    setParameters();

    // Provide all needed hardware to the ARM7
    // VRAM to ARM7 execution mode
    vramSetPrimaryBanks(VRAM_A_LCD, VRAM_B_LCD, VRAM_C_ARM7, VRAM_D_ARM7);
    // Slot 1 and Slot 2
    sysSetBusOwners(BUS_OWNER_ARM7, BUS_OWNER_ARM7);

    // Flush cache
    CP15_CleanAndFlushDCache();
    CP15_FlushICache();

    // Reboot the ARM7
    fifoSendValue32(FIFO_USER_01, MENU_MSG_ARM7_REBOOT_PICOLOADER);
    // And wait for it to receive the FIFO before killing all IRQs
    // Some arbitrary value should suffice?
    swiDelay(40);

    // Disable all IRQs
    irqDisable(IRQ_ALL);
    REG_IME = IME_DISABLE;
    REG_IE = 0;
    REG_IF = ~0;

    // Reboot to Pico Loader
    ((pico_loader_9_func_t)0x06800000)();

    // If we landed here this is a big problem
    return -1;
}