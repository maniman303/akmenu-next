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

#include <nds/ndstypes.h>

#include "PicoLoaderLauncher.h"
#include "../cheat.h"
#include "../dsrom.h"
#include "../flags.h"
#include "../inifile.h"
#include "../mainlist.h"
#include "../systemfilenames.h"
#include "../language.h"
#include "../logger.h"
#include "../ui/msgbox.h"
#include "../ui/progresswnd.h"
#include "NdsBootstrapLauncher.h"
#include "nds_loader_arm9.h"
#include "fsmngr.h"

#include "slot2/io_g6_common.h"
#include "slot2/io_m3_common.h"
#include "slot2/io_sc_common.h"

std::unique_ptr<TaskWorker> NdsBootstrapLauncher::task() const {
    return std::make_unique<NdsBootstrapLauncher>(*this);
}

bool NdsBootstrapLauncher::is3DS() {
    if (!isDSiMode()) {
        return false;
    }

    fifoSendValue32(FIFO_USER_01, MENU_MSG_IS_3DS);

    fifoWaitValue32(FIFO_USER_01);

    int result = fifoGetValue32(FIFO_USER_01);

    return result != 0;
}

bool NdsBootstrapLauncher::prepareCheats(const std::string& mRomPath) {
    u32 gameCode, crc32;
    std::string cheatDataPath = SFN_NDS_BOOTSTRAP_CHEATDATA;

    if (cCheat::romData(mRomPath, gameCode, crc32)) {
        std::string cheatFile = SFN_CHEATS;
        FILE* cheatDb = fopen(cheatFile.c_str(), "rb");
        if (!cheatDb) goto cheat_failed;
        long cheatOffset;
        size_t cheatSize;
        if (cCheat::searchCheatData(cheatDb, gameCode, crc32, cheatOffset, cheatSize)) {
            cCheat cheat;
            cheat.parse(mRomPath);
            cheat.writeCheatsToFile(cheatDataPath.c_str());
            FILE* cheatData = fopen(cheatDataPath.c_str(), "rb");
            if (cheatData) {
                u32 check[2];
                fread(check, 1, 8, cheatData);
                fclose(cheatData);
                // TODO: Delete file, if above 0x8000 bytes
                if (check[1] == 0xCF000000) goto cheat_failed;
            }
        } else {
            fclose(cheatDb);
            goto cheat_failed;
        }
        fclose(cheatDb);
    }

    return true;

cheat_failed:
    // Remove cheat bin if exists
    if (access(cheatDataPath.c_str(), F_OK) == 0) {
        remove(cheatDataPath.c_str());
    }

    return false;
}

bool NdsBootstrapLauncher::prepareIni(const std::string& mRomPath, const std::string& mSavePath, bool hb) {
    CIniFile ini;
    std::string iniPath = SFN_NDS_BOOTSTRAP_INI;
    bool hotkeyCheck = false;

    ini.SetString("NDS-BOOTSTRAP", "NDS_PATH", mRomPath);

    if (hb == true)
    {
        ini.SetString("NDS-BOOTSTRAP", "DSI_MODE", 0);
        ini.SaveIniFile(iniPath.c_str());
        return true;
    }

    ini.SetString("NDS-BOOTSTRAP", "SAV_PATH", mSavePath);

    ini.SetString("NDS-BOOTSTRAP", "QUIT_PATH", fsManager().resolveSystemPath("/_nds/akmenunext/launcher.nds"));

    ini.SetString("NDS-BOOTSTRAP", "CONSOLE_MODEL", is3DS() ? "2" : "0");

    std::string custIniPath = fsManager().resolveSystemPath("/_nds/akmenunext/ndsbs.ini");
    if (access(custIniPath.c_str(), F_OK) != 0) {
        akui::cMessageBox::showModal(LANG("nds bootstrap", "inimissingtitle"), LANG("nds bootstrap", "inimissing"), MB_OK);
        return false;
    }

    std::string externalHotkey;
    CIniFile extIni;
    if (extIni.LoadIniFile(custIniPath)) {
        externalHotkey = extIni.GetString("ndsbs", "hotkey", "");
        if (!externalHotkey.empty()) {
            hotkeyCheck = true;
        } else {
            hotkeyCheck = false;
        }
    } else {
        akui::cMessageBox::showModal(LANG("nds bootstrap", "inimissingtitle"), LANG("nds bootstrap", "inimissing"), MB_OK);
        return false;
    }

    /*
    0 = l-↓-select : 200 80 4
    1 = l-r-start : 200 100 8
    2 = l-r-select : 200 100 4
    3 = l-r-a-b-↓ : 200 100 1 2 80
    4 = l-r-b-y-↓ : 200 100 2 800 80
    5 = l-r-a-b-x-y : 200 100 1 2 400 800
    */

    switch(gs().resetHotKey)
    {
        case 0:
            ini.SetString("NDS-BOOTSTRAP", "HOTKEY", "284");
            break;
        case 1:
            ini.SetString("NDS-BOOTSTRAP", "HOTKEY", "308");
            break;
        case 2:
            ini.SetString("NDS-BOOTSTRAP", "HOTKEY", "304");
            break;
        case 3:
            ini.SetString("NDS-BOOTSTRAP", "HOTKEY", "383");
            break;
        case 4:
            ini.SetString("NDS-BOOTSTRAP", "HOTKEY", "B82");
            break;
        case 5:
            ini.SetString("NDS-BOOTSTRAP", "HOTKEY", "F03");
            break;
        case 6:
            if(hotkeyCheck){
                ini.SetString("NDS-BOOTSTRAP", "HOTKEY", externalHotkey.c_str());
            }
            break;
        default:
            break;
    }

    switch(gs().languageOverride)
    {
        case 0:
            ini.SetString("NDS-BOOTSTRAP", "LANGUAGE", "-1");
            break;
        case 1:
            ini.SetString("NDS-BOOTSTRAP", "LANGUAGE", "0");
            break;
        case 2:
            ini.SetString("NDS-BOOTSTRAP", "LANGUAGE", "1");
            break;
        case 3:
            ini.SetString("NDS-BOOTSTRAP", "LANGUAGE", "2");
            break;
        case 4:
            ini.SetString("NDS-BOOTSTRAP", "LANGUAGE", "3");
            break;
        case 5:
            ini.SetString("NDS-BOOTSTRAP", "LANGUAGE", "4");
            break;
        case 6:
            ini.SetString("NDS-BOOTSTRAP", "LANGUAGE", "5");
            break;
        case 7:
            ini.SetString("NDS-BOOTSTRAP", "LANGUAGE", "6");
            break;
        case 8:
            ini.SetString("NDS-BOOTSTRAP", "LANGUAGE", "7");
            break;
    }

    if (gs().dsOnly) {
        ini.SetString("NDS-BOOTSTRAP", "DSI_MODE", "0");
    }
    
    if (gs().phatCol && isDSiMode()) {
        ini.SetString("NDS-BOOTSTRAP", "PHAT_COLORS", "1");
    }

    if (access("/_nds/debug.txt", F_OK) == 0) {
        ini.SetString("NDS-BOOTSTRAP", "LOGGING", "1");
        ini.SetString("NDS-BOOTSTRAP", "DEBUG", "1");
    }

    ini.SaveIniFile(iniPath.c_str());
    return true;
}

bool NdsBootstrapLauncher::process() {
    static const std::string ndsBootstrapCheck = fsManager().resolveSystemPath("/_nds/pagefile.sys");
    std::string bootstrapDir = SFN_NDS_BOOTSTRAP_DIR;

    switch (_iter) {
      case 0:
        _messageBlock = false;
        _ndsBootstrapPath = SFN_NDS_BOOTSTRAP_EXEC;
        _argv.clear();

        if (access(ndsBootstrapCheck.c_str(), F_OK) != 0) {
            _messageBlock = true;
            akui::cMessageBox::showModal(LANG("nds bootstrap", "firsttimetitle"), LANG("nds bootstrap", "firsttime"), MB_OK, [this](){
                _messageBlock = false;
            });
        }

        _iter++;
        return false;
      case 1:
        if (_messageBlock) {
            return false;
        }

        _romInfo.MayBeDSRom(_romPath);
        if ((gs().nightly && _romInfo.saveInfo().getNightly() == 2) || _romInfo.saveInfo().getNightly() == 1) {
            _ndsBootstrapPath = SFN_NDS_BOOTSTRAP_EXEC_NIGHTLY;
        }

        if (access(_ndsBootstrapPath.c_str(), F_OK) != 0) {
            progressWnd().hide();
            fprintf(stderr, "No access to the '%s' file.", _ndsBootstrapPath.c_str());
            showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), _ndsBootstrapPath.c_str()));
            return true;
        }

        progressWnd().setTipText(LANG("loader", "nds init"));
        progressWnd().show();
        progressWnd().setPercent(0);

        _iter++;
        return false;
      case 2:
        // Create the nds-bootstrap directory if it doesn't exist
        if (access(bootstrapDir.c_str(), F_OK) != 0) {
            mkdir(bootstrapDir.c_str(), 0777);
        }

        progressWnd().setPercent(33);

        _iter++;
        return false;
      case 3:
        // Setup argv to launch nds-bootstrap
        _argv.push_back(_ndsBootstrapPath.c_str());

        // Prepare cheat codes if enabled
        if (_flags & PATCH_CHEATS) {
            if (!prepareCheats(_mRomPath)) {
                return true;
            }

            progressWnd().setTipText(LANG("loader", "cheats"));
            progressWnd().setPercent(66);
        }
        
        _iter++;
        return false;
      case 4:
        // Setup nds-bootstrap INI parameters
        if (!prepareIni(_mRomPath, _mSavePath, _hb)) {
            return true;
        }

        progressWnd().setTipText(LANG("loader", "boot"));
        progressWnd().setPercent(100);

        logger().info("Running nds bootstrap.");

        PicoLoaderLauncher().launchRom(_ndsBootstrapPath, "", _flags, 0, 0, true);

        progressWnd().hide();
        return true;
    }

    return true;
}
