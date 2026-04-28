/*
    mainwnd.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "mainwnd.h"
#include "../../share/fifotool.h"
#include "dbgtool.h"
#include "msgbox.h"
#include "systemfilenames.h"
#include "timer.h"
#include "timetool.h"
#include "windowmanager.h"
#include "diskicon.h"

#include "datetime.h"
#include "logger.h"

#include "expwnd.h"
#include "favorites.h"
#include "gbaloader.h"
#include "helpwnd.h"
#include "cachedinifile.h"
#include "language.h"
#include "progresswnd.h"
#include "romlauncher.h"
#include "ticksound.h"
#include "vfxmanager.h"

#include <dirent.h>
#include <fat.h>
#include <sys/iosupport.h>

#include "launcher/HomebrewLauncher.h"
#include "launcher/NdsBootstrapLauncher.h"
#include "launcher/PassMeLauncher.h"
#include "launcher/Slot1Launcher.h"

cMainWnd::cMainWnd(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text)
    : cForm(x, y, w, h, parent, text),
      _mainList(NULL),
      _startMenu(NULL),
      _startButton(NULL),
      _brightnessButton(NULL),
      _clockButton(NULL),
      _folderUpButton(NULL),
      _folderText(NULL),
      _focusBorder(NULL) {}

cMainWnd::~cMainWnd() {
    if (_folderText != NULL) {
        delete _folderText;
    }
    
    if (_folderUpButton != NULL) {
        delete _folderUpButton;
    }

    if (_brightnessButton != NULL) {
        delete _brightnessButton;
    }

    if (_startButton != NULL) {
        delete _startButton;
    }

    if (_startMenu != NULL) {
        delete _startMenu;
    }

    if (_mainList != NULL) {
        delete _mainList;
    }

    if (_focusBorder != NULL) {
        delete _focusBorder;
    }

    windowManager().removeWindow(this);
}

void cMainWnd::init() {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    bool focus = false;
    bool show = false;
    COLOR color = 0;
    std::string file("");
    std::string text("");
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);

    // self init
    dbg_printf("mainwnd init() %08x\n", this);
    loadAppearance(SFN_LOWER_SCREEN_BG);

    // init game file list
    // waitMs( 2000 );
    _mainList = new cMainList(this, "main list");
    _mainList->init();
    _mainList->rowClicked.connect(this, &cMainWnd::onMainListSelItemClicked);
    _mainList->directoryChanged.connect(this, &cMainWnd::onFolderChanged);
    addChildWindow(_mainList);

    // waitMs( 1000 );

    // init start button
    x = ini.GetInt("start button", "x", 0);
    y = ini.GetInt("start button", "y", 172);
    focus = ini.GetInt("start button", "focus", 0);
    show = ini.GetInt("start button", "show", 1);
    color = ini.GetInt("start button", "textColor", 0x7fff);
    file = ini.GetString("start button", "file", "none");
    text = ini.GetString("start button", "text", "ini");
    if (file != "none") {
        file = SFN_UI_CURRENT_DIRECTORY + file;
    }
    if (text == "ini") {
        text = LANG("start menu", "START");
    }

    _startButton = new cButton(x, y, 48, 10, this, text);
    _startButton->setStyle(cButton::press);
    _startButton->setRelativePosition(cPoint(x, y));
    _startButton->setIsFocusable(focus);
    _startButton->loadAppearance(file);
    _startButton->clicked.connect(this, &cMainWnd::startButtonClicked);
    _startButton->setTextColor(color | BIT(15));
    if (!show) _startButton->hide();
    addChildWindow(_startButton);

    // init brightness button
    x = ini.GetInt("brightness btn", "x", 240);
    y = ini.GetInt("brightness btn", "y", 1);
    focus = ini.GetInt("brightness btn", "focus", 0);
    show = ini.GetInt("brightness btn", "show", 1);
    _brightnessButton = new cButton(x, y, 0, 0, this, "");
    _brightnessButton->setRelativePosition(cPoint(x, y));
    _brightnessButton->setIsFocusable(focus);
    _brightnessButton->loadAppearance(SFN_BRIGHTNESS_BUTTON);
    _brightnessButton->clicked.connect(this, &cMainWnd::brightnessButtonClicked);
    if (!show) _brightnessButton->hide();
    addChildWindow(_brightnessButton);

    x = ini.GetInt("clock btn", "x", 240);
    y = ini.GetInt("clock btn", "y", 1);
    focus = ini.GetInt("clock btn", "focus", 0);
    show = ini.GetInt("clock btn", "show", 0);
    _clockButton = new cButton(x, y, 0, 0, this, "");
    _clockButton->setRelativePosition(cPoint(x, y));
    _clockButton->setIsFocusable(focus);
    _clockButton->loadAppearance(SFN_CLOCK_BUTTON);
    _clockButton->clicked.connect(this, &cMainWnd::clockButtonClicked);
    if (!show) _clockButton->hide();
    addChildWindow(_clockButton);

    x = ini.GetInt("folderup btn", "x", 0);
    y = ini.GetInt("folderup btn", "y", 2);
    w = ini.GetInt("folderup btn", "w", 32);
    h = ini.GetInt("folderup btn", "h", 16);
    _folderUpButton = new cButton(x, y, w, h, this, "");
    _folderUpButton->setRelativePosition(cPoint(x, y));
    _folderUpButton->loadAppearance(SFN_FOLDERUP_BUTTON);
    _folderUpButton->setSize(cSize(w, h));
    _folderUpButton->clicked.connect(_mainList, &cMainList::backParentDir);
    addChildWindow(_folderUpButton);

    x = ini.GetInt("folder text", "x", 20);
    y = ini.GetInt("folder text", "y", 2);
    w = ini.GetInt("folder text", "w", 160);
    h = ini.GetInt("folder text", "h", 16);
    _folderText = new cStaticText(x, y, w, h, this, "");
    _folderText->setRelativePosition(cPoint(x, y));
    _folderText->setTextColor(ini.GetInt("folder text", "color", 0));
    addChildWindow(_folderText);

    // init startmenu
    _startMenu = new cStartMenu(160, 40, 61, 108, NULL, "start menu");
    _startMenu->init();
    _startMenu->itemClicked.connect(this, &cMainWnd::startMenuItemClicked);

    dbg_printf("startMenu %08x\n", _startMenu);

    _focusBorder = new cFocusBorder(this);
    _focusBorder->init();

    diskIcon().loadAppearance(SFN_CARD_ICON_BLUE);
    diskIcon().setParent(this);
    addChildWindow(&diskIcon());

    cFavorites::RemoveInvalidFavorites();
}

void cMainWnd::update() {
    _focusBorder->update();
}

void cMainWnd::draw() {
    cForm::draw();
    _focusBorder->draw(selectedEngine());
}

void cMainWnd::startMenuItemClicked(s16 i) {
    dbg_printf("start menu item %d\n", i);

    if (START_MENU_ITEM_FAVORITES == i) {
        std::string selectedFullPath = _mainList->getRowFullPath(_mainList->selectedRowId());
        bool favoritesRes = false;
        if (cFavorites::IsInFavorites(selectedFullPath)) {
            favoritesRes = cFavorites::RemoveFromFavorites(selectedFullPath);
        } else {
            favoritesRes = cFavorites::AddToFavorites(selectedFullPath);
        }

        if (favoritesRes) {
            _mainList->enterDir(_mainList->getCurrentDir());
        }
    }

    else if (START_MENU_ITEM_INFO == i) {
        showFileInfo(_mainList->selectedRowId());
    }

    else if (START_MENU_ITEM_SETTING == i) {
        showSettings();
    }

    else if (START_MENU_ITEM_HELP == i) {
        cHelpWnd::showModal(this);
    } else if (START_MENU_ITEM_TOOLS == i) {
        cExpWnd::showModal(this);
    }
}

void cMainWnd::startButtonClicked() {
    if (!gs().safeMode) {
        WorkIndicatorTask* task = new WorkIndicatorTask({_focusBorder}, this, [this]() {
            _startMenu->showForFile(_mainList->getRowFullPath(_mainList->selectedRowId()));
            windowManager().addWindow(_startMenu);
        });
        task->schedule();
    }
}

void cMainWnd::brightnessButtonClicked() {
    gs().nextBrightness();
}

void cMainWnd::clockButtonClicked() {
    std::string lastFile = saveManager().lastFile();
    if (lastFile == "..." || lastFile.empty()) {
        return;
    }

    u32 selectedId = _mainList->getRowIdByPath(lastFile);
    if (selectedId >= UINT16_MAX) {
        return;
    }

    showFileInfo(selectedId);
}

cWindow& cMainWnd::loadAppearance(const std::string& aFileName) {
    return *this;
}

void cMainWnd::onFocused() {
    cForm::onFocused();
    windowManager().setFocusedWindow(_mainList);
}

bool cMainWnd::processKeyMessage(cKeyMessage message) {
    bool isL = message.isKeyShift(KEY_L);
    bool allow = !gs().safeMode;
    if (message.isKeyUp(KEY_Y)) {
        if (isL) {
            showSettings();
        } else if (!gs().safeMode) {
            showFileInfo(_mainList->selectedRowId());
        }

        return true;
    }

    if (message.isKeyUp(KEY_X)) {
        if (isL) {
            if (allow) {
                DSRomInfo rominfo;
                if (_mainList->getRomInfo(_mainList->selectedRowId(), rominfo) &&
                    rominfo.isDSRom() && !rominfo.isHomebrew()) {
                    cRomInfoWnd::showCheats(_mainList->getRowFullPath(_mainList->selectedRowId()));
                }
            }
        } else {
            _mainList->enterDir("favorites:/");
            _mainList->selectRow(0);
        }

        return true;
    }

    if (message.isKeyUp(KEY_START)) {
        startButtonClicked();
        return true;
    }

    if (message.isKeyUp(KEY_SELECT)) {
        if (!allow) {
            return true;
        }

        if (isL) {
            _mainList->SwitchShowAllFiles();
            return true;
        }
    }

    if (message.isKeyUp(KEY_R)) {
        if (isL) {
            brightnessButtonClicked();
        }

        return true;
    }

    return cForm::processKeyMessage(message);
}

bool cMainWnd::processTouchMessage(cTouchMessage message) {
    return cForm::processTouchMessage(message);
}

void cMainWnd::onMainListSelItemClicked(u32 index) {
    WorkIndicatorTask* task = new WorkIndicatorTask({_focusBorder}, this, [this](){
        launchSelected();
    });
    
    task->schedule();
}

void cMainWnd::setFocusedChild(cWindow* child) {
    WorkIndicatorTask* task = new WorkIndicatorTask({_focusBorder}, this, [this, child]() {
        windowManager().setFocusedWindow(child);
    });

    task->schedule();
}

void cMainWnd::launchSelected() {
    u32 selectedRowId = _mainList->selectedRowId();
    std::string fullPath = _mainList->getRowFullPath(selectedRowId);
    std::string romName = _mainList->getRowShowName(selectedRowId);
    std::string fileName = _mainList->getRowFileName(selectedRowId);
    size_t lastSlashPos = fullPath.find_last_of("/\\");
    std::string directory = fullPath.substr(0, lastSlashPos + 1);
    
    // Create the new path by appending "saves/"
    std::string savesPath = formatString("%ssaves/%s", directory.c_str(), fileName.c_str());

    if (fullPath[fullPath.size() - 1] == '/') {
        _mainList->enterDir(fullPath);
        return;
    }

    DSRomInfo rominfo;
    if (!_mainList->getRomInfo(_mainList->selectedRowId(), rominfo)) return;

    // rominfo.loadDSRomInfo( fullPath, false );

    if (rominfo.isGbaRom()) {
        CGbaLoader(fullPath).Load(false, false);
        return;
    }

    if (!rominfo.isDSRom()) {
        return;
    }

    bool isMenu = rominfo.isHomebrew() && "boot.nds" == toLowerString(fileName);

    disableInput();
    tickSound().disable();
    vfxManager().playEffect(VFX_EFFECT::SELECT);
    WorkIndicatorTask* task = new WorkIndicatorTask({_focusBorder, &vfxManager()}, this, [this, fullPath, rominfo, isMenu, fileName, savesPath](){
        TLaunchResult launchRes = launchRom(fullPath, rominfo, isMenu, savesPath, [this](){
            enableInput();
            tickSound().enable();
        });

        if (launchRes == ELaunchNoFreeSpace) {
            akui::cMessageBox::showModal(LANG("no free space", "title"), LANG("no free space", "text"), MB_OK);
        }
    });
    task->schedule();
}

void cMainWnd::showSettings(void) {
    // logger().info("Start prepping settings.");

    if (gs().safeMode) {
        return;
    }

    cSettingWnd* settingWnd = new cSettingWnd(LANG("start menu", "Setting"), "main_settings", [this](cSettingWnd* wnd) { saveSettings(wnd); });
    settingWnd->addSettingTab(LANG("system setting", "title"));

    std::vector<std::string> _values;
    
    // page 1: system
    // language
    u32 langIndex = 0;
    _values = fsManager().getLangNames();
    for (size_t ii = 0; ii < _values.size(); ++ii) {
        if (_values[ii] == gs().langDirectory) langIndex = ii;
    }
    settingWnd->addSettingItem(LANG("language", "text"), _values, langIndex);

    _values.clear();
    _values.push_back(LANG("date format", "dd-mm-yyyy"));
    _values.push_back(LANG("date format", "mm-dd-yyyy"));
    settingWnd->addSettingItem(LANG("date format", "title"), _values, gs().dateFormat);

    _values.clear();
    _values.push_back(LANG("clock format", "24 hour"));
    _values.push_back(LANG("clock format", "12 hour"));
    settingWnd->addSettingItem(LANG("clock format", "text"), _values, gs().show12hrClock);

    _values.clear();
    _values.push_back(LANG("switches", "Disable"));
    _values.push_back(LANG("switches", "Enable"));
    settingWnd->addSettingItem(LANG("quick resume", "text"), _values, gs().autorunWithLastRom);

    // page 2: interface
    settingWnd->addSettingTab(LANG("interface settings", "title"));

    // user interface style
    u32 uiIndex = 0;
    std::string currentUIStyle = gs().uiName;
    _values = fsManager().getUiNames();
    for (size_t ii = 0; ii < _values.size(); ++ii) {
        if (_values[ii] == gs().uiName) uiIndex = ii;
    }
    settingWnd->addSettingItem(LANG("ui style", "text"), _values, uiIndex);

    size_t scrollSpeed = 0;
    switch (gs().scrollSpeed) {
        case cGlobalSettings::EScrollFast:
            scrollSpeed = 0;
            break;
        case cGlobalSettings::EScrollMedium:
            scrollSpeed = 1;
            break;
        case cGlobalSettings::EScrollSlow:
            scrollSpeed = 2;
            break;
    }
    _values.clear();
    _values.push_back(LANG("scrolling", "fast"));
    _values.push_back(LANG("scrolling", "medium"));
    _values.push_back(LANG("scrolling", "slow"));
    settingWnd->addSettingItem(LANG("interface settings", "scrolling speed"), _values, scrollSpeed);

    _values.clear();
    _values.push_back(LANG("menu style", "full"));
    _values.push_back(LANG("menu style", "user only"));
    _values.push_back(LANG("menu style", "games"));
    settingWnd->addSettingItem(LANG("menu style", "menu"), _values, gs().filePresentationMode);

    _values.clear();
    _values.push_back(LANG("interface settings", "oldschool"));
    _values.push_back(LANG("interface settings", "modern"));
    _values.push_back(LANG("interface settings", "internal"));
    _values.push_back(LANG("interface settings", "small"));
    settingWnd->addSettingItem(LANG("menu style", "item"), _values, gs().viewMode);

    // page 3: filesystem
    settingWnd->addSettingTab(LANG("file settings", "title"));
    
    // file list type
    _values.clear();
    for (size_t ii = 0; ii < 3; ++ii) {
        std::string itemName = formatString("item%d", ii);
        _values.push_back(LANG("filelist type", itemName));
    }
    settingWnd->addSettingItem(LANG("filelist type", "text"), _values, gs().fileListType);
    
    _values.clear();
    _values.push_back(".nds.sav");
    _values.push_back(".sav");
    settingWnd->addSettingItem(LANG("file settings", "save extension"), _values, gs().saveExt);

    _values.clear();
    _values.push_back(LANG("message box", "no"));
    _values.push_back(LANG("message box", "yes"));
    settingWnd->addSettingItem(LANG("file settings", "use saves folder"), _values, gs().saveDir);

    // page 4: ndsbs
    settingWnd->addSettingTab(LANG("setting window", "patches"));
    _values.clear();
    _values.push_back(LANG("switches", "Disable"));
    _values.push_back(LANG("switches", "Enable"));
    settingWnd->addSettingItem(LANG("nds bootstrap", "dsmode"), _values, gs().dsOnly);
    _values.clear();
    _values.push_back(LANG("nds bootstrap", "release"));
    _values.push_back(LANG("nds bootstrap", "nightly"));
    settingWnd->addSettingItem(LANG("nds bootstrap", "text"), _values, gs().nightly);

    _values.clear();
    _values.push_back(LANG("override", "0"));
    _values.push_back(LANG("override", "1"));
    _values.push_back(LANG("override", "2"));
    _values.push_back(LANG("override", "3"));
    _values.push_back(LANG("override", "4"));
    _values.push_back(LANG("override", "5"));
    _values.push_back(LANG("override", "6"));
    _values.push_back(LANG("override", "7"));
    _values.push_back(LANG("override", "8"));
    settingWnd->addSettingItem(LANG("override", "text"), _values, gs().languageOverride);

    // reset hotkey
    _values.clear();
    _values.push_back(LANG("resethotkey", "0"));
    _values.push_back(LANG("resethotkey", "1"));
    _values.push_back(LANG("resethotkey", "2"));
    _values.push_back(LANG("resethotkey", "3"));
    _values.push_back(LANG("resethotkey", "4"));
    _values.push_back(LANG("resethotkey", "5"));
    _values.push_back(LANG("resethotkey", "6"));
    settingWnd->addSettingItem(LANG("resethotkey", "text"), _values, gs().resetHotKey);

    if (fsManager().isFlashcart()){
        _values.clear();
        _values.push_back("nds-bootstrap");
        _values.push_back("Pico-Loader");
        settingWnd->addSettingItem(LANG("nds bootstrap", "loader"), _values, gs().pico);
    }

#ifdef __KERNEL_LAUNCHER_SUPPORT__
    _values.clear();
    _values.push_back("Kernel");
    _values.push_back("nds-bootstrap");
    settingWnd.addSettingItem(LANG("loader", "text"), _values, gs().romLauncher);
#endif

    // page 5: other
    settingWnd->addSettingTab(LANG("gba settings", "title"));
    _values.clear();
    _values.push_back(LANG("switches", "Disable"));
    _values.push_back(LANG("switches", "Enable"));
    settingWnd->addSettingItem(LANG("interface settings", "clock sound"), _values, gs().clockSound);

    _values.clear();
    _values.push_back(LANG("switches", "Disable"));
    _values.push_back(LANG("switches", "Enable"));
    settingWnd->addSettingItem(LANG("patches", "cheating system"), _values, gs().cheats);

    _values.clear();
    _values.push_back(LANG("gba settings", "modeask"));
    _values.push_back(LANG("gba settings", "modegba"));
    _values.push_back(LANG("gba settings", "modends"));
    settingWnd->addSettingItem(LANG("gba settings", "mode"), _values, gs().slot2mode);

    if (isDSiMode()) {
        _values.clear(); 
        _values.push_back(LANG("patches", "default"));
        _values.push_back(LANG("patches", "ndshb"));
        settingWnd->addSettingItem(LANG("patches", "hbstrap"), _values, gs().hbStrap);
    }

    windowManager().addModal(settingWnd);
}

void cMainWnd::saveSettings(cSettingWnd* settingWnd) {
    if (settingWnd == NULL) {
        return;
    }

    u8 currentFileListType = gs().fileListType;
    int currentfilePresentationMode = gs().filePresentationMode;

    u32 uiIndex = 0;
    std::vector<std::string> uiNames = fsManager().getUiNames();
    for (size_t ii = 0; ii < uiNames.size(); ++ii) {
        if (uiNames[ii] == gs().uiName) uiIndex = ii;
    }

    u32 langIndex = 0;
    std::vector<std::string> langNames = fsManager().getLangNames();
    for (size_t ii = 0; ii < langNames.size(); ++ii) {
        if (langNames[ii] == gs().langDirectory) langIndex = ii;
    }
    
    // page 1: system
    u32 langIndexAfter = settingWnd->getItemSelection(0, 0);
    gs().dateFormat = settingWnd->getItemSelection(0, 1);
    gs().show12hrClock = settingWnd->getItemSelection(0, 2);
    gs().autorunWithLastRom = settingWnd->getItemSelection(0, 3);

    // page 2: interface
    u32 uiIndexAfter = settingWnd->getItemSelection(1, 0);

    switch (settingWnd->getItemSelection(1, 1)) {
        case 0:
            gs().scrollSpeed = cGlobalSettings::EScrollFast;
            break;
        case 1:
            gs().scrollSpeed = cGlobalSettings::EScrollMedium;
            break;
        case 2:
            gs().scrollSpeed = cGlobalSettings::EScrollSlow;
            break;
    }
    gs().filePresentationMode = settingWnd->getItemSelection(1, 2);
    gs().viewMode = settingWnd->getItemSelection(1, 3);

    // page 3: filesystem
    gs().fileListType = settingWnd->getItemSelection(2, 0);
    gs().saveExt = settingWnd->getItemSelection(2, 1);
    gs().saveDir = settingWnd->getItemSelection(2, 2);

    // page 4: ndsbs
    gs().dsOnly = settingWnd->getItemSelection(3, 0);
    gs().nightly = settingWnd->getItemSelection(3, 1);
    gs().languageOverride = settingWnd->getItemSelection(3,2);
    gs().resetHotKey = settingWnd->getItemSelection(3, 3);

    if (fsManager().isFlashcart()) {
        gs().pico = settingWnd->getItemSelection(3, 4);
    }

    // page 5: other
    gs().clockSound = settingWnd->getItemSelection(4, 0);
    gs().cheats = settingWnd->getItemSelection(4, 1);
    gs().slot2mode = settingWnd->getItemSelection(4, 2);

    if (isDSiMode()){
        gs().hbStrap = settingWnd->getItemSelection(4, 3);
    }

    if (uiIndex != uiIndexAfter) {
        akui::cMessageBox::showModal(LANG("ui style changed", "title"), LANG("ui style changed", "text"), MB_YES | MB_NO,
            [this, uiNames, uiIndexAfter, langNames, langIndexAfter]() {
                gs().uiName = uiNames[uiIndexAfter];
                gs().langDirectory = langNames[langIndexAfter];
                gs().saveSettings();

                disableInput();
                std::string launcherPath = fsManager().resolveSystemPath("/_nds/akmenunext/launcher.nds");
                HomebrewLauncher* launcher = new HomebrewLauncher();
                launcher->setOnCompleted([this](){
                    enableInput();
                });
                launcher->launchRom(launcherPath, "", 0, 0, 0, 0);
            },
            {});
    } else if (langIndex != langIndexAfter) {
        akui::cMessageBox::showModal(LANG("language changed", "title"), LANG("language changed", "text"), MB_YES | MB_NO,
            [this, langNames, langIndexAfter]() {
                gs().langDirectory = langNames[langIndexAfter];
                gs().saveSettings();

                disableInput();
                std::string launcherPath = fsManager().resolveSystemPath("/_nds/akmenunext/launcher.nds");
                HomebrewLauncher* launcher = new HomebrewLauncher();
                launcher->setOnCompleted([this](){
                    enableInput();
                });
                launcher->launchRom(launcherPath, "", 0, 0, 0, 0);
            },
            {});
    }

    gs().saveSettings();
    _mainList->setViewMode((cMainList::VIEW_MODE)gs().viewMode);

    if (gs().filePresentationMode != currentfilePresentationMode) {
        _mainList->enterDir("...");
        _mainList->selectRow(0);
    } else if (gs().fileListType != currentFileListType) {
        _mainList->enterDir(_mainList->getCurrentDir());
    }
}

void cMainWnd::showFileInfo(u32 id) {
    u32 selectedRomId = id;
    DSRomInfo rominfo;
    if (!_mainList->getRomInfo(selectedRomId, rominfo)) {
        return;
    }

    std::string showName = _mainList->getRowShowName(selectedRomId);
    std::string fullPath = _mainList->getRowFullPath(selectedRomId);
    cRomInfoWnd* romInfoWnd = new cRomInfoWnd(LANG("rom info", "title"), [this](cRomInfoWnd* wnd) { saveFileInfo(wnd); });
    romInfoWnd->setFileInfo(fullPath, showName);
    romInfoWnd->setRomInfo(rominfo);
    romInfoWnd->setSaves(_mainList->Saves());
    
    windowManager().addModal(romInfoWnd);
}

void cMainWnd::saveFileInfo(cRomInfoWnd* romInfoWnd) {
    if (romInfoWnd == NULL) {
        return;
    }

    DSRomInfo rominfo = romInfoWnd->getRomInfo();
    _mainList->setRomInfo(_mainList->selectedRowId(), rominfo);
}

void cMainWnd::onFolderChanged() {
    std::string dirShowName = _mainList->getCurrentDir();
    if (dirShowName == "favorites:/") {
        _folderText->setText(dirShowName);

        return;
    }

    u32 selectedRowId = _mainList->selectedRowId();
    if (_mainList->getRowFullPath(selectedRowId) == "slot2:/") {
        u8 chk = CGbaLoader::GetGbaHeader();
        if (chk != GBA_HEADER.complement) {
            std::string title = LANG("no gba card", "title");
            std::string text = LANG("no gba card", "text");
            akui::cMessageBox::showModal(title, text, MB_OK);
            _mainList->enterDir("...");
            _mainList->selectRow(_mainList->Slot2());
            return;
        }

        WorkIndicatorTask* task = new WorkIndicatorTask({_focusBorder}, this, [this]() {
            int mode = gs().slot2mode;
            if (mode == cGlobalSettings::ESlot2Ask) {
                akui::cMessageBox::showModal(LANG("gba settings", "mode"), LANG("gba settings", "modetext"), MB_YES_NO,
                    [this]() {
                        disableInput();
                        PassMeLauncher* launcher = new PassMeLauncher();
                        launcher->setOnCompleted([this](){
                            enableInput();
                        });
                        launcher->launchRom("slot2:/", "", 0, 0, 0, 0);
                    },
                    [this]() {
                        CGbaLoader::StartGBA();
                    });
            } else if (mode == cGlobalSettings::ESlot2Nds) {
                disableInput();
                PassMeLauncher* launcher = new PassMeLauncher();
                launcher->setOnCompleted([this](){
                    enableInput();
                });
                launcher->launchRom("slot2:/", "", 0, 0, 0, 0);
            } else {
                CGbaLoader::StartGBA();
            }
        });
        task->schedule();
    }

    if (_mainList->getRowFullPath(selectedRowId) == "slot1:/") {
        disableInput();
        tickSound().disable();
        vfxManager().playEffect(VFX_EFFECT::SELECT);
        WorkIndicatorTask* task = new WorkIndicatorTask({_focusBorder, &vfxManager()}, this, [this](){
            Slot1Launcher* launcher = new Slot1Launcher();
            launcher->setOnCompleted([this](){
                enableInput();
                tickSound().enable();
            });
            launcher->launchRom("slot1:/", "", 0, 0, 0, 0);
        });
        task->schedule();
    }

    _folderText->setText(dirShowName);
}
