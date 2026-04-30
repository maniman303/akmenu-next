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
#include "logger.h"

#include "mainlist.h"
#include "mainwnd.h"
#include "rombootwnd.h"
#include "startmenu.h"
#include "systemfilenames.h"
#include "timer.h"
#include "systemdetails.h"

#include "analogclock.h"
#include "bigclock.h"
#include "calendar.h"
#include "calendarwnd.h"
#include "datetime.h"
#include "batterymeter.h"
#include "booticon.h"
#include "fpscounter.h"
#include "image.h"

#include "vfxmanager.h"
#include "ticksound.h"

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
#include "bmp15.h"

#include "ui/image.h"
#include "ui/windowmanager.h"
#include "ui/subwindowmanager.h"
#include "ui/uisettings.h"

#include "tasks/taskcruncher.h"
#include "tasks/screenfade.h"

#ifdef __cplusplus
extern "C" {
#endif
void __libnds_exit(int rc) {}
#ifdef __cplusplus
}
#endif

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

void initMainWindow() {
    akui::cImage* background = new akui::cImage(NULL, cSize(SCREEN_WIDTH, SCREEN_HEIGHT), 0xffff);    
    background->loadAppearance(SFN_LOWER_SCREEN_BG);
    windowManager().addWindow(background);

    cMainWnd* wnd = new cMainWnd(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, "main window");
    wnd->init();

    std::string lastDirectory = saveManager().lastDirectory();
    std::string lastFile = saveManager().lastFile();

    if (gs().filePresentationMode < 2) {
        if (wnd->_mainList->enterDir(lastDirectory != "..." ? lastDirectory : gs().startupFolder)) {
            wnd->_mainList->selectRom(lastFile);
        } else {
            wnd->_mainList->enterDir("...");
        }
    } else {
        wnd->_mainList->enterDir("...");

        if (lastFile != "..." && !lastFile.empty()) {
            wnd->_mainList->selectRom(lastFile);
        }
    }

    windowManager().addWindow(wnd);
    tickSound().enable();
    
    ScreenFadeTask* fadeTask = new ScreenFadeTask();
    fadeTask->schedule();
}

int main(int argc, char* argv[]) {
    consoleDebugInit(DebugDevice_NOCASH);

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

    // init graphics
    gdi().init();

    sd().initArm7RegStatuses();

    fsManager().init(argc, argv);

    logger().init();

    consoleDebugInit(DebugDevice_NOCASH);

    // setting scripts
    gs().loadSettings();

    saveManager().loadLastInfo();

    vfxManager().init();
    
    cwl();

    lang();  // load language file
    gs().language = lang().GetInt("font", "language", gs().language);
    fontFactory().makeFont();  // load font file
    uiSettings().loadSettings();

    bool saveListOK = saveManager().importSaveList(SFN_CUSTOM_SAVELIST, SFN_OFFICIAL_SAVELIST);
    if (!saveListOK) {
        dbg_printf("WARNING: savelist.bin missed\n");
    }

    sd().update();

    // Backup save data from chip and gba sram save data to flash.
    if (gs().gbaAutoSave && expansion().IsValid()) {
        saveSram();
    }

    progressWnd().init();

    subWindowManager().init();
    fpsCounter().init();

    if (gs().autorunWithLastRom && saveManager().lastFile() != "..." && !saveManager().lastFile().empty()) {
        cRomBootWnd* romBoot = new cRomBootWnd(saveManager().lastFile(), []() {
            initMainWindow();    
        });

        windowManager().addModal(romBoot);

        ScreenFadeTask* fadeTask = new ScreenFadeTask(true, false, true);
        fadeTask->schedule();
    } else {
        initMainWindow();
    }

    irq().vblankStart();

    *(u32*)(0xCFFFD0C) = 0x454D4D43;
    while (*(u32*)(0xCFFFD0C) != 0) {
        swiDelay(100);
    }

    u16 ticks = 0;
    while (true) {
        timer().updateFrames();
        tickSound().play();

        if (ticks % 15 == 0) {
            sd().update();
        }

        INPUT& inputs = updateInput();
        processInput(inputs);

        taskCruncher().process();

        windowManager().update();
        subWindowManager().update();

        datetime().purge();
        destroyBMP15();

        swiWaitForVBlank();
        gdi().present();

        if (ticks >= 60) {
            ticks = 0;
        } else {
            ticks++;
        }
    }

    return 0;
}
