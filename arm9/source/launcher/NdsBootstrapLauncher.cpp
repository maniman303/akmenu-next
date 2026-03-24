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

#include "../cheatwnd.h"
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

void slot2RamAccess(){
    // if running from NDS slot
    if (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS) {
        sysSetCartOwner(BUS_OWNER_ARM9);

        *(vu16*)0x08000000 = 0x1111;    // write data to GBA ROM area
        // if data wasn't written, try enabling RAM access for various slot 2 carts and write again
        if (*(vu16*)0x08000000 != 0x1111) {
            _SC_changeMode(SC_MODE_RAM);
            *(vu16*)0x08000000 = 0x1111;
        }
        if (*(vu16*)0x08000000 != 0x1111) {
            _G6_SelectOperation(G6_MODE_RAM);
            *(vu16*)0x08000000 = 0x1111;
        }
        if (*(vu16*)0x08000000 != 0x1111) {
            _M3_changeMode(M3_MODE_RAM);
            *(vu16*)0x08000000 = 0x1111;
        }

        sysSetCartOwner(BUS_OWNER_ARM7);
    }
}

std::unique_ptr<TaskWorker> NdsBootstrapLauncher::task() const {
    return std::make_unique<NdsBootstrapLauncher>(*this);
}

bool NdsBootstrapLauncher::prepareCheats(const std::string& mRomPath) {
    u32 gameCode, crc32;

    if (cCheatWnd::romData(mRomPath, gameCode, crc32)) {
        FILE* cheatDb = fopen((SFN_CHEATS).c_str(), "rb");
        if (!cheatDb) goto cheat_failed;
        long cheatOffset;
        size_t cheatSize;
        if (cCheatWnd::searchCheatData(cheatDb, gameCode, crc32, cheatOffset, cheatSize)) {
            cCheatWnd chtwnd((256) / 2, (192) / 2, 100, 100, NULL, mRomPath);

            chtwnd.parse(mRomPath);
            chtwnd.writeCheatsToFile("/_nds/nds-bootstrap/cheatData.bin");
            FILE* cheatData = fopen("/_nds/nds-bootstrap/cheatData.bin", "rb");
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
    if (access("/_nds/nds-bootstrap/cheatData.bin", F_OK) == 0) {
        remove("/_nds/nds-bootstrap/cheatData.bin");
    }

    return false;
}

bool NdsBootstrapLauncher::prepareIni(const std::string& mRomPath, const std::string& mSavePath, bool hb) {
    CIniFile ini;
    bool hotkeyCheck = false;

    ini.SetString("NDS-BOOTSTRAP", "NDS_PATH", mRomPath);

    if(hb == true)
    {
        ini.SetString("NDS-BOOTSTRAP", "DSI_MODE", 0);
        ini.SaveIniFile("/_nds/nds-bootstrap.ini");
        return true;
    }

    ini.SetString("NDS-BOOTSTRAP", "SAV_PATH", mSavePath);

    ini.SetString("NDS-BOOTSTRAP", "QUIT_PATH", fsManager().resolveSystemPath("/_nds/akmenunext/launcher.nds"));

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

    ini.SaveIniFile("/_nds/nds-bootstrap.ini");

    return true;
}

bool launchHbStrap(std::string romPath){
    progressWnd().setPercent(100);
    progressWnd().hide();

    std::string ndsHbBootstrapPath = fsManager().resolveSystemPath("/_nds/nds-bootstrap-hb-release.nds");
    std::vector<const char*> argv;
    argv.push_back(ndsHbBootstrapPath.c_str());
    eRunNdsRetCode rc = runNdsFile(argv[0], argv.size(), &argv[0]);
    if (rc == RUN_NDS_OK) {
        return true;
    }

    return false;
}

bool NdsBootstrapLauncher::process() {
    static const std::string ndsBootstrapCheck = fsManager().resolveSystemPath("/_nds/pagefile.sys");
    static const std::string ndsBootstrapPath = fsManager().resolveSystemPath("/_nds/nds-bootstrap-release.nds");
    static const std::string ndsBootstrapPathNightly = fsManager().resolveSystemPath("/_nds/nds-bootstrap-nightly.nds");
    static const std::string ndsHbBootstrapPath = fsManager().resolveSystemPath("/_nds/nds-bootstrap-hb-release.nds");

    bool isDsiWare = false;

    // logger().info("Starting nds-bootstrap with iter: " + std::to_string(_iter) + ".");

    switch (_iter) {
      case 0:
        _messageBlock = false;
        _useNightly = false;
        _argv.clear();

        // logger().info("Nds-bootstrap case 0.");

        if (access(ndsBootstrapCheck.c_str(), F_OK) != 0) {
            // logger().info("Nds-bootstrap case 0 no access.");

            _messageBlock = true;
            akui::cMessageBox::showModal(LANG("nds bootstrap", "firsttimetitle"), LANG("nds bootstrap", "firsttime"), MB_OK, [this](){
                _messageBlock = false;
            });
        }

        // logger().info("Nds-bootstrap case 0 completed.");

        _iter++;
        return false;
      case 1:
        if (_messageBlock) {
            return false;
        }

        //check if rom is homebrew
        if (_hb) {
            if (access(ndsHbBootstrapPath.c_str(), F_OK) != 0) {
                progressWnd().hide();
                showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), ndsHbBootstrapPath.c_str()));
                return true;
            }

            progressWnd().setTipText("Initializing nds-bootstrap...");
            progressWnd().show();

            //Clean up old INI
            if (access("/_nds/nds-bootstrap/nds-bootstrap.ini", F_OK) == 0) {
                remove("/_nds/nds-bootstrap/nds-bootstrap.ini");
            }

            // Setup nds-bootstrap INI parameters
            if (!prepareIni(_romPath, _savePath, false)) {
                progressWnd().hide();
                return true;
            }

            progressWnd().setPercent(25);
        }

        _iter++;
        return false;
      case 2:
        if (_hb) {
            launchHbStrap(_romPath);

            return true;
        }

        _romInfo.MayBeDSRom(_romPath);
        isDsiWare = _romInfo.isDSiWare();

        // check for DSiWare
        if (isDsiWare) {
            progressWnd().hide();
            akui::cMessageBox::showModal(LANG("loader", "error"), LANG("loader", "dsi pico"), MB_OK);
            return true;
        }

        progressWnd().setTipText(LANG("loader", "nds init"));
        progressWnd().show();
        progressWnd().setPercent(0);

        _iter++;
        return false;
      case 3:
        //Check which nds-bootstrap version has been selected
        if ((gs().nightly && _romInfo.saveInfo().getNightly() == 2) || _romInfo.saveInfo().getNightly() == 1) {
            if (access(ndsBootstrapPathNightly.c_str(), F_OK) != 0){
                progressWnd().hide();
                showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), ndsBootstrapPathNightly.c_str()));
                return true;
            } else {
                _useNightly = true;
            }
        } else if (access(ndsBootstrapPath.c_str(), F_OK) != 0) {
            progressWnd().hide();
            showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), ndsBootstrapPath.c_str()));
            return true;
        }
        else {
            _useNightly = false;
        }

        // Create the nds-bootstrap directory if it doesn't exist
        if (access("/_nds/nds-bootstrap/", F_OK) != 0) {
            mkdir("/_nds/nds-bootstrap/", 0777);
        }

        progressWnd().setPercent(25);

        _iter++;
        return false;
      case 4:
        // Setup argv to launch nds-bootstrap                             
        if (!_useNightly){
            _argv.push_back(ndsBootstrapPath.c_str());
        } else {
            _argv.push_back(ndsBootstrapPathNightly.c_str());
        }

        progressWnd().setPercent(50);

        _iter++;
        return false;
      case 5:
        //Clean up old INI
        if (access("/_nds/nds-bootstrap/nds-bootstrap.ini", F_OK) == 0) {
            remove("/_nds/nds-bootstrap/nds-bootstrap.ini");
        }

        // Setup nds-bootstrap INI parameters
        if (!prepareIni(_romPath, _savePath, false)) {
            progressWnd().hide();
            return true;
        }

        progressWnd().setPercent(75);

        _iter++;
        return false;
      case 6:
        // Prepare cheat codes if enabled
        // Remove cheat bin if exists to start clean
        if (access("/_nds/nds-bootstrap/cheatData.bin", F_OK) == 0) {
            remove("/_nds/nds-bootstrap/cheatData.bin");
        }

        if (!_romInfo.saveInfo().getCheat() || !gs().cheats) {
            _iter++;
            return false;
        }

        progressWnd().setTipText("Loading usrcheat.dat...");
        progressWnd().setPercent(90);
        if (_flags & PATCH_CHEATS && !prepareCheats(_romPath)) {
            progressWnd().hide();
            return true;
        }

        _iter++;
        return false;
      case 7:
        progressWnd().setTipText("Booting game...");
        progressWnd().setPercent(100);

        _iter++;
        return false;
      case 8:
        // enable slot2 ram access if available
        if(!isDSiMode()){
            slot2RamAccess();
        }

        // Launch
        eRunNdsRetCode rc = runNdsFile(_argv[0], _argv.size(), &_argv[0]);
        if (rc == RUN_NDS_OK) return true;
        return true;
    }

    return true;
}
