/*
    Copyright (C) 2024 lifehackerhansol
    Additional modifications Copyright (C) 2025 coderkei

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <fat.h>
#include <nds/arm9/dldi.h>

#include <nds/ndstypes.h>

#include "DSpicoLauncher.h"
#include "tonccpy.h"
#include "../cheatwnd.h"
#include "../flags.h"
#include "../mainlist.h"
#include "../systemfilenames.h"
#include "../language.h"
#include "../ui/msgbox.h"
#include "../ui/progresswnd.h"

constexpr std::align_val_t cache_align { 32 };

typedef void (*pico_loader_9_func_t)(void);

std::unique_ptr<TaskWorker> DSpicoLauncher::task() const {
    return std::make_unique<DSpicoLauncher>(*this);
}

bool DSpicoLauncher::process() {
    static const char picoLoader7Path[] = "fat:/_pico/picoLoader7.bin";
    static const char picoLoader9Path[] = "fat:/_pico/picoLoader9.bin";

    switch (_iter) {
      case 0:
        _loader7 = NULL;
        _loader9 = NULL;
        _sLoadParams = {};
        _picoLoader7Size = 0;
        _picoLoader9Size = 0;
        _picoLoader7 = NULL;
        _picoLoader9 = NULL;

        progressWnd().setTipText("Initializing pico-loader...");
        progressWnd().show();
        progressWnd().setPercent(0);

        _iter++;
        return false;
     case 1:
        if (access(picoLoader7Path, F_OK) != 0) {
            progressWnd().hide();
            showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), picoLoader7Path));
            return true;
        }

        if (access(picoLoader9Path, F_OK) != 0) {
            progressWnd().hide();
            showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), picoLoader9Path));
            return true;
        }

        _loader7 = fopen(picoLoader7Path, "rb");
        if(_loader7 == NULL) {
            progressWnd().hide();
            showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), picoLoader7Path));
            return true;
        }

        _loader9 = fopen(picoLoader9Path, "rb");
        if (_loader9 == NULL) {
            fclose(_loader7);
            progressWnd().hide();
            showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), picoLoader9Path));
            return true;
        }

        strcpy(_sLoadParams.romPath, _romPath.c_str());
        strcpy(_sLoadParams.savePath, _savePath.c_str());

        fseek(_loader7, 0, SEEK_END);
        _picoLoader7Size = ftell(_loader7);
        fseek(_loader7, 0, SEEK_SET);

        fseek(_loader9, 0, SEEK_END);
        _picoLoader9Size = ftell(_loader9);
        fseek(_loader9, 0, SEEK_SET);

        progressWnd().setPercent(40);

        _iter++;
        return false;
      case 2:
        _picoLoader7 = new(cache_align) u8[_picoLoader7Size];
        fread(_picoLoader7, 1, _picoLoader7Size, _loader7);
        fclose(_loader7);

        _picoLoader9 = new(cache_align) u8[_picoLoader9Size];
        fread(_picoLoader9, 1, _picoLoader9Size, _loader9);
        fclose(_loader9);

        progressWnd().setPercent(75);

        _iter++;
        return false;
      case 3:
        soundDisable();
        irqDisable(IRQ_ALL & ~(IRQ_FIFO_EMPTY |IRQ_FIFO_NOT_EMPTY));

        vramSetBankA(VRAM_A_LCD);
        vramSetBankB(VRAM_B_LCD);
        vramSetBankC(VRAM_C_LCD);
        vramSetBankD(VRAM_D_LCD);

        tonccpy((void*)0x06800000, _picoLoader9, (_picoLoader9Size + 1) & ~1);
        tonccpy((void*)0x06840000, _picoLoader7, (_picoLoader7Size + 1) & ~1);
        delete[] _picoLoader9;
        delete[] _picoLoader7;
        _picoLoader9 = NULL;
        _picoLoader7 = NULL;

        //clear out ARM9 DMA channels
        for (int i = 0; i < 4; i++) {
            DMA_CR(i) = 0;
            DMA_SRC(i) = 0;
            DMA_DEST(i) = 0;
            TIMER_CR(i) = 0;
            TIMER_DATA(i) = 0;
        }

        DC_FlushAll();
        DC_InvalidateAll();
        IC_InvalidateAll();
        sysSetBusOwners(false, false);

        ((pload_header7_t*)0x06840000)->bootDrive = PLOAD_BOOT_DRIVE_DLDI;
        ((pload_header7_t*)0x06840000)->dldiDriver = (void*)io_dldi_data;
        tonccpy(&((pload_header7_t*)0x06840000)->loadParams, &_sLoadParams, sizeof(pload_params_t));
        vramSetBankC(VRAM_C_ARM7_0x06000000);
        vramSetBankD(VRAM_D_ARM7_0x06020000);
        fifoSendValue32(FIFO_USER_01, MENU_MSG_ARM7_REBOOT_PICOLOADER);
        irqDisable(IRQ_ALL);

        ((pico_loader_9_func_t)0x06800000)();
        return true;
    }

    return true;
}