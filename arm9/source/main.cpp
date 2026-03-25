/*
    main.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <nds.h>
#include <nds/arm9/dldi.h>
#include <cstdio>
#include <list>
#include <map>
#include <vector>

#include <fat.h>

#include "dbgtool.h"
#include "gdi.h"
#include "ui.h"
#include "logger.h"

#include "mainlist.h"
#include "mainwnd.h"
#include "startmenu.h"
#include "systemfilenames.h"
#include "timer.h"
#include "systemdetails.h"

#include "bigclock.h"
#include "calendar.h"
#include "calendarwnd.h"
#include "datetime.h"
#include "batterymeter.h"
#include "screenoverlay.h"

#include "ticksound.h"
#include "taskcruncher.h"

#include "smalldate.h"
#include "smallclock.h"

#include "inifile.h"
#include "irqs.h"

#include "fontfactory.h"
#include "language.h"
#include "progresswnd.h"

#include "exptools.h"
#include "romlauncher.h"
#include "sram.h"
#include "userwnd.h"
#include "fsmngr.h"

using namespace akui;

#ifdef __cplusplus
extern "C" {
#endif
void __libnds_exit(int rc) {}
#ifdef __cplusplus
}
#endif

static bool runAutoLoop = false;

void saveSram() {
    CIniFile f;
    if (!f.LoadIniFile(SFN_LAST_GBA_SAVEINFO)) {
        return;
    }

    std::string psramFile = f.GetString("Save Info", "lastLoaded", "");
    if (psramFile != "") {
        cSram::SaveSramToFile(psramFile.c_str(), cExpansion::EPsramPage);
        f.SetString("Save Info", "lastLoaded", "");
        f.SaveIniFile(SFN_LAST_GBA_SAVEINFO);
    }

    std::string norFile = f.GetString("Save Info", "lastLoadedNOR", "");
    if (norFile.empty()) {
        return; 
    }
    
    std::string norFileSave = norFile + ".sav";
    FILE* saveFile = fopen(norFileSave.c_str(), "rb");
    if (saveFile == NULL) {
        return;
    }

    cSram::sSaveInfo saveInfo;
    cSram::ProcessRAW(saveFile, saveInfo);
    u8* bufFile = (u8*)malloc(saveInfo.size);
    if (bufFile) {
        memset(bufFile, 0, saveInfo.size);
        fread(bufFile, saveInfo.size, 1, saveFile);
        u8* bufData = cSram::SaveSramToMemory(cExpansion::ENorPage, saveInfo, false);
        if (bufData) {
            if (memcmp(bufFile, bufData, saveInfo.size) != 0) {
                cSram::SaveSramToFile(norFile.c_str(), cExpansion::ENorPage);
            }

            free(bufData);
        }
        
        free(bufFile);
    }

    fclose(saveFile);
}

int main(int argc, char* argv[]) {
    irq().init();

    windowManager();

    // init basic system
    sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);

    // init tick timer/fps counter
    timer().initTimer();

    // init inputs
    initInput();

    // turn led on
    ledBlink(PM_LED_ON);

    // soundEnable();

    // init graphics
    gdi().init();

    sd().initArm7RegStatuses();

    fsManager().init(argc, argv);

    logger().init();

    // setting scripts
    gs().loadSettings();

    saveManager().loadLastInfo()
    
    cwl();

    lang();  // load language file
    gs().language = lang().GetInt("font", "language", gs().language);
    fontFactory().makeFont();  // load font file
    uiSettings().loadSettings();

    bool saveListOK = saveManager().importSaveList(SFN_CUSTOM_SAVELIST, SFN_OFFICIAL_SAVELIST);
    if (!saveListOK) {
        dbg_printf("WARNING: savelist.bin missed\n");
    }

    tickSound().load(SFN_UI_TICK_SOUND);
    sd().update();

    // set last directory
    std::string lastDirectory = "...", lastFile = "...";
    if (gs().enterLastDirWhenBoot || gs().autorunWithLastRom) {
        lastFile = saveManager().getLastInfo();
        if (lastFile.empty()) {
            lastFile = "...";
        } else if (gs().enterLastDirWhenBoot && gs().filePresentationMode < 2) {
            size_t slashPos = lastFile.find_last_of('/');
            if (lastFile.npos != slashPos) lastDirectory = lastFile.substr(0, slashPos + 1);
        }
    }

    // Backup save data from chip and gba sram save data to flash.
    if (gs().gbaAutoSave && expansion().IsValid()) {
        saveSram();
    }

    gdi().initBg("", true);
    progressWnd().init();

    if (!fsManager().isRebooted() && gs().autorunWithLastRom && lastFile != "..." && !lastFile.empty()) {
        INPUT& inputs = updateInput();
        if (!(inputs.keysHeld & KEY_B)) {
            runAutoLoop = true;
            autoLaunchRom(lastFile, []() {
                runAutoLoop = false;
            });

            windowManager().addWindow(&screenOverlay());

            while (runAutoLoop) {
                swiWaitForVBlank();

                inputs = updateInput();
                processInput(inputs);
                
                taskCruncher().process();
                windowManager().update();

                gdi().present(GE_MAIN);
            }
        }
    }

    windowManager().removeWindow(&screenOverlay());

    calendarWnd().init();
    calendar().init();
    bigClock().init();
    batteryMeter().init();

    smallDate().init();
    smallClock().init();

    cMainWnd* wnd = new cMainWnd(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, "main window");
    wnd->init();

    timer().updateFps();

    irq().vblankStart();

    dbg_printf("lastDirectory '%s'\n", lastDirectory.c_str());
    if (!wnd->_mainList->enterDir("..." != lastDirectory ? lastDirectory : gs().startupFolder))
        wnd->_mainList->enterDir("...");

    u16 ticks = 0;
    bool isBgInit = false;

    *(u32*)(0xCFFFD0C) = 0x454D4D43;
    while (*(u32*)(0xCFFFD0C) != 0) {
        swiDelay(100);
    }

    while (true) {
        timer().updateFps();
        tickSound().play();

        if (ticks == 0) {
            sd().update();
        }

        swiWaitForVBlank();

        INPUT& inputs = updateInput();
        processInput(inputs);

        if (ticks >= 29) {
            ticks = 0;
        } else {
            ticks++;
        }
        
        taskCruncher().process();
        windowManager().update();

        irq().drawTop();

        if (!isBgInit) {
            gdi().initBg(SFN_LOWER_SCREEN_BG, false);
            isBgInit = true;
        }

        gdi().present(GE_SUB);
        gdi().present(GE_MAIN);
    }

    return 0;
}
