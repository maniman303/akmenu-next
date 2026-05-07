/*
    gbaloader.cpp
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <string.h>
#include "gbaloader.h"
#include "exptools.h"
#include "bmp15.h"
#include "systemfilenames.h"
#include "sram.h"
#include "fsmngr.h"
#include "logger.h"
#include "savemngr.h"
#include "launcher/nds_loader_arm9.h"
#include "../../share/fifotool.h"

cGbaLoader::cGbaLoader() {
    _isRunnerPresent = false;
    _isBiosPresent = false;
}

void cGbaLoader::init() {
    _isRunnerPresent = fsManager().fileExists(SFN_GBARUNNER_NDS);
    _isBiosPresent = fsManager().fileExists(SFN_GBARUNNER_BIOS);
}

bool cGbaLoader::validate() {
    return _isRunnerPresent && _isBiosPresent;
}

bool cGbaLoader::startRom(const std::string& fileName) {
    std::string gbaRunnerNds = SFN_GBARUNNER_NDS;

    if (!_isRunnerPresent) {
        logger().error("File '" + gbaRunnerNds + "' could not be found.");
        return false;
    }

    if (!_isBiosPresent) {
        logger().error("File '" + SFN_GBARUNNER_BIOS + "' could not be found.");
        return false;
    }

    // TODO: Generate gbarunner3 settings

    if (!tryCopyBorder()) {
        logger().error("File '" + SFN_GBAFRAME + "' could not be copied.");
    }

    saveManager().saveLastInfo(fileName);

    std::vector<const char*> argv;
    argv.push_back(gbaRunnerNds.c_str());
    argv.push_back(fileName.c_str());

    eRunNdsRetCode res = runNdsFile(argv[0], argv.size(), &argv[0]);

    return res == eRunNdsRetCode::RUN_NDS_OK;
}

bool cGbaLoader::tryCopyBorder() {
    std::string themeBorder = SFN_GBAFRAME;
    std::string runnerBorder = SFN_GBARUNNER_FRAME;

    FILE* src = fopen(themeBorder.c_str(), "rb");
    if (!src) {
        return false;
    }

    FILE* dst = fopen(runnerBorder.c_str(), "wb");
    if (!dst) {
        fclose(src);
        return false;
    }

    char buffer[4096];

    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytesRead, dst) != bytesRead) {
            fclose(src);
            fclose(dst);
            return false;
        }
    }

    fclose(src);
    fclose(dst);

    return true;
}

void cGbaLoader::StartGBA(void) {
    LoadBorder();
    BootGBA();
    while (true) swiWaitForVBlank();
}

void cGbaLoader::LoadBorder(void) {
    videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    vramSetPrimaryBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
                        VRAM_C_SUB_BG_0x06200000, VRAM_D_LCD);

    // for the main screen
    REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_WRAP_OFF;
    REG_BG3PA = 1 << 8;  // scale x
    REG_BG3PB = 0;       // rotation x
    REG_BG3PC = 0;       // rotation y
    REG_BG3PD = 1 << 8;  // scale y
    REG_BG3X = 0;        // translation x
    REG_BG3Y = 0;        // translation y

    memset((void*)BG_BMP_RAM(0), 0, 0x18000);
    memset((void*)BG_BMP_RAM(8), 0, 0x18000);

    cBMP15 frameBMP = createBMP15FromFile(SFN_GBAFRAME);
    if (frameBMP.valid() && frameBMP.width() == SCREEN_WIDTH &&
        frameBMP.height() == SCREEN_HEIGHT) {
        DC_FlushRange(frameBMP.buffer(), SCREEN_WIDTH * SCREEN_HEIGHT * 2);
        dmaCopy(frameBMP.buffer(), (void*)BG_BMP_RAM(0), SCREEN_WIDTH * SCREEN_HEIGHT * 2);
        dmaCopy(frameBMP.buffer(), (void*)BG_BMP_RAM(8), SCREEN_WIDTH * SCREEN_HEIGHT * 2);
    }
}

void cGbaLoader::BootGBA(void) {
    sysSetBusOwners(BUS_OWNER_ARM7, BUS_OWNER_ARM7);
    if (PersonalData->gbaScreen)
        REG_POWERCNT = 1;
    else
        REG_POWERCNT = (POWER_SWAP_LCDS | 1) & 0xffff;
    fifoSendValue32(FIFO_USER_01, MENU_MSG_GBA);
}

u8 cGbaLoader::GetGbaHeader() {
    u8 chk = 0;
    for (u32 i = 0xA0; i < 0xBD; ++i) {
        chk = chk - *(u8*)(0x8000000 + i);
    }
    chk = (chk - 0x19) & 0xff;

    return chk;
}
