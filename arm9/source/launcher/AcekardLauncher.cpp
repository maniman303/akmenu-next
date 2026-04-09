/*
    AcekardLauncher.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <climits>
#include <cstring>
#include <string>

#include <nds.h>

#include "../../../share/fifotool.h"
#include "../ui/progresswnd.h"
#include "AcekardLauncher.h"
#include "dbgtool.h"

// 256 UCS-2 characters encoded into UTF-8 can use up to 768 UTF-8 chars
#define MAX_FILENAME_LENGTH 768

static void resetAndLoop() {
    DC_FlushAll();
    DC_InvalidateAll();

    fifoSendValue32(FIFO_USER_01, MENU_MSG_ARM7_REBOOT);
    *((vu32*)0x02FFFE04) = 0;

    // Interrupt
    REG_IME = 0;
    REG_IE = 0;
    REG_IF = ~0;

    // wait for arm7
    while (*((vu32*)0x02FFFE04) == 0)
        ;
    swiSoftReset();
}

std::unique_ptr<TaskWorker> AcekardLauncher::task() const {
    return std::make_unique<AcekardLauncher>(*this);
}

s16 AcekardLauncher::process(s16 iter) {
    tNDSHeader header;

    dbg_printf("load %s\n", _romPath.c_str());

    std::string acekardLoaderPath = "fat:/__rpg/akloader.nds";

    if (access(acekardLoaderPath.c_str(), F_OK) != 0) {
        showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), acekardLoaderPath.c_str()));
        return -1;
    }

    // akloaders are very old and expect the old-style libfat mount points
    _romPath.replace(0, 3, "fat0");
    _savePath.replace(0, 3, "fat0");

#ifdef __AKLOADER_AK2__
    // Create TTMENU.SYS if it don't exist
    if (access("fat:/__rpg/system.sys", F_OK) != 0) {
        FILE* TTSYSFile = fopen("fat:/__rpg/system.sys", "wb");
        if (TTSYSFile == NULL) {
            showModalOk(LANG("loader", "error"), LANG("loader", "ttmenu"));
            return true;
        }

        fseek(TTSYSFile, 0, SEEK_SET);
        // memdump. Actually just expanding the file seems to crash, but this works totally fine...
        fwrite((void*)0x02400000, 1, 0x400000, TTSYSFile);
        fflush(TTSYSFile);
        fclose(TTSYSFile);
    }
#endif

    FILE* loader = fopen(acekardLoaderPath.c_str(), "rb");
    if (loader == NULL) {
        return -1;
    }

    fseek(loader, 0, SEEK_SET);
    fread(&header, sizeof(header), 1, loader);

    // copy loader's arm7 code
    fseek(loader, header.arm7romOffset, SEEK_SET);
    fread(header.arm7destination, 4, header.arm7binarySize >> 2, loader);
    __NDSHeader->arm7executeAddress = header.arm7executeAddress;

    // copy loader's arm9 code
    fseek(loader, header.arm9romOffset, SEEK_SET);
    fread(header.arm9destination, 4, header.arm9binarySize >> 2, loader);
    __NDSHeader->arm9executeAddress = header.arm9executeAddress;
    fclose(loader);

    *(u32*)0x23fd900 = _flags;
    *(u32*)0x23fd904 = _cheatOffset;
    *(u32*)0x23fd908 = _cheatSize;
    memset((void*)0x23fda00, 0, MAX_FILENAME_LENGTH * 2);
    strcpy((char*)0x23fda00, _romPath.c_str());
    strcpy((char*)(0x23fda00 + MAX_FILENAME_LENGTH), _savePath.c_str());

    dbg_printf("load done\n");

    resetAndLoop();
    return -1;
}
