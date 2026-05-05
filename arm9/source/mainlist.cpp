/*
    mainlist.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "mainlist.h"
#include <fat.h>
#include <sys/dir.h>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include "dbgtool.h"
#include "gbaloader.h"
#include "folder_banner_bin.h"
#include "gba_banner_bin.h"
#include "cachedinifile.h"
#include "language.h"
#include "microsd_banner_bin.h"
#include "nand_banner_bin.h"
#include "nds_save_banner_bin.h"
#include "progresswnd.h"
#include "startmenu.h"
#include "systemfilenames.h"
#include "timetool.h"
#include "unicode.h"
#include "unknown_banner_bin.h"
#include "windowmanager.h"
#include "fsmngr.h"
#include "favorites.h"
#include "savemngr.h"
#include "logger.h"
#include "ui/msgbox.h"
#include "tasks/directoryload.h"
#include "../../share/memtool.h"

cMainList::cMainList(cWindow* parent, const std::string& text)
    : cListView(4, 20, 248, 152, parent, text) {
    _textPrefix = 0;
    _textSufix = 0;
    _iconPrefix = 1;
    _iconSufix = 3;
    _tallRowHeight = 38;
    _centerInternalColumn = false;
    _viewMode = VM_LIST;
    _busy = false;
    _scheduledRom = "";
    _canRenderBackdrop = true;
    _scheduleBackdrop = true;
}

cMainList::~cMainList() {
    if (_itemBg) {
        delete _itemBg;
    }
}

int cMainList::init() {
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _iconPrefix = ini.GetInt("main list", "iconPrefix", 1);
    _iconSufix = ini.GetInt("main list", "iconSufix", 3);
    _textPrefix = ini.GetInt("main list", "textPrefix", 0);
    _textSufix = ini.GetInt("main list", "textSufix", 0);
    _textColor = ini.GetInt("main list", "textColor", RGB15(7, 7, 7));
    _textColorHilight = ini.GetInt("main list", "textColorHilight", RGB15(31, 0, 31));
    _selectionBarColor1 = ini.GetInt("main list", "selectionBarColor1", RGB15(16, 20, 24));
    _selectionBarColor2 = ini.GetInt("main list", "selectionBarColor2", RGB15(20, 25, 0));
    _selectionBarOpacity = ini.GetInt("main list", "selectionBarOpacity", 100);
    _tallRowHeight = ini.GetInt("main list", "tallRowHeight", 38);
    _centerInternalColumn = ini.GetInt("main list", "centerInternalColumn", false);

    s32 x = ini.GetInt("main list", "x", 4);
    s32 y = ini.GetInt("main list", "y", 20);
    setRelativePosition(cPoint(x, y));

    s32 w = ini.GetInt("main list", "w", 248);
    s32 h = ini.GetInt("main list", "h", 152);
    h = (h / _tallRowHeight) * _tallRowHeight;
    setSize(cSize(w, h));

    _itemBg = new akui::cImage(this);
    _itemBg->loadAppearance(SFN_MAIN_LIST_ITEM_BG);

    insertColumn(ICON_COLUMN, "icon", 0);
    insertColumn(SHOWNAME_COLUMN, "showName", 0);
    insertColumn(INTERNALNAME_COLUMN, "internalName", 0, _centerInternalColumn);
    insertColumn(REALNAME_COLUMN, "realName", 0);  // hidden column for contain real filename
    insertColumn(SAVETYPE_COLUMN, "saveType", 0);
    insertColumn(FILESIZE_COLUMN, "fileSize", 0);
    insertColumn(IS_FAVORITE_COLUMN, "isFavorite", 0);

    setViewMode((cMainList::VIEW_MODE)gs().viewMode);

    return 1;
}

bool cMainList::insertEntryRow(const std::vector<std::string>& texts) {
    size_t index = getRowCount();
    std::vector<std::string> copy(texts);
    while (copy.size() < IS_FAVORITE_COLUMN) {
        copy.push_back("");
    }

    if (copy.size() <= IS_FAVORITE_COLUMN) {
        copy.push_back("false");
    }
    
    if (!insertRow(index, copy)) {
        return false;
    }

    return true;
}

void cMainList::processDirIcon(DSRomInfo& romInfo, const std::string fileName) {
    if (fileName == "fat:/" || fileName == "sd:/") {
        romInfo.setFileName(fileName);
        romInfo.setBannerFromFile(fsManager().getIconPath("microsd_banner.bin"), microsd_banner_bin);
        return;
    }

    if (fileName == "favorites:/") {
        romInfo.setFileName(fileName);
        romInfo.setBannerFromFile(fsManager().getIconPath("folder_banner.bin"), folder_banner_bin);
        return;
    }

    if (fileName == "slot1:/") {
        romInfo.setFileName(fileName);
        romInfo.setBannerFromFile(fsManager().getIconPath("nand_banner.bin"), nand_banner_bin);
        return;
    }

    if (fileName == "slot2:/") {
        romInfo.setFileName(fileName);
        romInfo.setBannerFromFile(fsManager().getIconPath("gba_banner.bin"), gba_banner_bin);
        return;
    }

    if (fileName.back() == '/') {
        romInfo.setFileName(fileName);
        romInfo.setBannerFromFile(fsManager().getIconPath("folder_banner.bin"), folder_banner_bin);
        return;
    }

    std::string extName = "";
    size_t lastDotPos = fileName.find_last_of('.');
    if (fileName.npos != lastDotPos) {
        extName = toLowerString(fileName.substr(lastDotPos));
    }

    if (extName == ".sav") {
        romInfo.setFileName(fileName);
        romInfo.setBannerFromFile(fsManager().getIconPath("nds_save_banner.bin"), nds_save_banner_bin);
        return;
    }

    if (extName == ".nds") {
        romInfo.mayBeDSRom(fileName);
        return;
    }
    
    if (extName == ".gba") {
        romInfo.mayBeGbaRom(fileName);
        return;
    }
    
    romInfo.setFileName(fileName);
    romInfo.setBannerFromFile(fsManager().getIconPath("unknown_banner.bin"), unknown_banner_bin);
}

void cMainList::processDirIcons() {
    u32 total = onScreenRowCount();

    _romInfoList.clear();
    _romInfoList.resize(total);

    for (size_t i = 0; i < total; i++) {
        DSRomInfo& romInfo = _romInfoList[(_firstVisibleRowId + i) % total];
        std::string fileName = _rows[_firstVisibleRowId + i][REALNAME_COLUMN].text();
        
        processDirIcon(romInfo, fileName);
    }
}

void cMainList::validateDirIcons() {
    u32 total = onScreenRowCount();

    for (size_t i = 0; i < total; i++) {
        DSRomInfo& romInfo = _romInfoList[(_firstVisibleRowId + i) % total];
        std::string fileName = _rows[_firstVisibleRowId + i][REALNAME_COLUMN].text();
        
        if (romInfo.fileName() == fileName) {
            continue;
        }

        _romInfoList[(_firstVisibleRowId + i) % total] = DSRomInfo();
        romInfo = _romInfoList[(_firstVisibleRowId + i) % total];
        processDirIcon(romInfo, fileName);
    }
}

void cMainList::onScrolled(u32 index) {
    validateDirIcons();
}

void cMainList::onDirectoryChanged(std::deque<std::vector<std::string>>& rows, std::string dirName) {
    bool changed = _currentDir != dirName;

    if (_parent != NULL) {
        _parent->enableInput();
    }
    
    _currentDir = dirName;
    _busy = false;

    removeAllRows();
    _romInfoList.clear();

    while (!rows.empty()) {
        std::vector<std::string>& row = rows.front();
        insertEntryRow(row);
        rows.pop_front();
    }

    processDirIcons();
    scheduleBackdrop();
    
    if (changed) {
        directoryChanged();
    }
}

bool cMainList::enterDir(const std::string& dirName) {
    return enterDir(dirName, {});
}

bool cMainList::enterDir(const std::string& dirName, std::function<void()> onCompleted) {
    if (dirName == "slot2:/") {
        _currentDir = "";
        directoryChanged();
        return true;
    }

    if (dirName == "slot1:/") {
        _currentDir = "";
        directoryChanged();
        return true;
    }

    std::string tempDirName = dirName;
    if (startsWithString(tempDirName, "...") || tempDirName.empty()) {
        tempDirName = "...";
    } else if (tempDirName != "favorites:/") {
        DIR* dir = opendir(tempDirName.c_str());

        if (dir == NULL) {
            if (fsManager().getFSRoot() == tempDirName) {
                std::string title = LANG("sd card error", "title");
                std::string sdError = LANG("sd card error", "text");
                akui::cMessageBox::showModal(title, sdError, MB_OK);
            }

            return false;
        }

        closedir(dir);
    }

    if (_parent != NULL) {
        _parent->disableInput();
    }

    _busy = true;

    DirectoryLoadTask* task = new DirectoryLoadTask(tempDirName, [this, tempDirName](std::deque<std::vector<std::string>> rows) {
        onDirectoryChanged(rows, tempDirName);
    });

    task->setOnCompleted(onCompleted);
    task->schedule();

    return true;
}

std::string cMainList::processItemText(std::string text, int column) {
    if (column != SHOWNAME_COLUMN && column != INTERNALNAME_COLUMN) {
        return text;
    }

    if (gs().filePresentationMode == 0) {
        return text;
    }

    text = replaceInString(text, "; ", ": ");
    if (text == "saves") {
        return "Saves";
    }

    size_t lastdot = text.find_last_of(".");
    if (lastdot == std::string::npos) {
        return text;
    }

    std::string extName = "";
    if (text.npos != lastdot) {
        extName = toLowerString(text.substr(lastdot));
    }

    if (extName != ".nds" && extName != ".sav" && extName != ".gba") {
        return text;
    }

    return text.substr(0, lastdot);
}

bool cMainList::backParentDir() {
    if (_currentDir == "...") {
        return false;
    }

    bool fat1 = fsManager().getFSRoot() == _currentDir;
    bool favorites = "favorites:/" == _currentDir;
    if (_currentDir == "fat:/" || _currentDir == "sd:/" || fat1 || favorites || _currentDir == "/") {
        enterDir("...");
        if (fat1) {
            scheduleRomSelection("fat:/");
        } else if (favorites) {
            scheduleRomSelection("favorites:/");
        } else {
            scheduleRomSelection("...");
        }

        return true;
    }

    size_t pos = _currentDir.rfind("/", _currentDir.size() - 2);
    std::string parentDir = _currentDir.substr(0, pos + 1);
    std::string oldCurrentDir = _currentDir;

    if (enterDir(parentDir)) {  // select last entered director
        scheduleRomSelection(oldCurrentDir);

        return true;
    }

    return false;
}

void cMainList::update() {
    if (_busy) {
        return;
    }

    if (!_scheduledRom.empty()) {
        selectRom(_scheduledRom, true);
        _scheduledRom = "";
    }
}

bool cMainList::processKeyMessage(cKeyMessage message) {
    if (message.isKeyUp(KEY_B)) {
        if (backParentDir()) {
            directoryReturned();
        }

        return true;
    }

    return cListView::processKeyMessage(message);
}

cRect cMainList::focusRectangle() const {
    if (_rows.size() == 0) {
        return cRect(position(), size(), false);
    }

    u32 visibleRowId = _selectedRowId - _firstVisibleRowId;
    cPoint rowPos = position() + cPoint(0, (visibleRowId * _rowHeight) - 1);
    cSize rowSize = cSize(size().x, _rowHeight + 1);

    return cRect(rowPos, rowSize, false);
}

std::string cMainList::getRowFullPath(u32 id) const {
    if (!_rows.size() || id >= (u32)_rows.size()) {
        return std::string("");
    }

    return _rows[id][REALNAME_COLUMN].text();
}

std::string cMainList::getRowShowName(u32 id) const {
    if (!_rows.size() || id >= (u32)_rows.size()) {
        return std::string("");
    }

    return _rows[id][SHOWNAME_COLUMN].text();
}

std::string cMainList::getRowFileName(u32 id) const {
    if (!_rows.size() || id >= (u32)_rows.size()) {
        return std::string("");
    }

    std::string fullPath = _rows[id][REALNAME_COLUMN].text();
    size_t lastSlashPos = fullPath.find_last_of("/\\");

    return fullPath.substr(lastSlashPos + 1);
}

u32 cMainList::getRowIdByPath(std::string path) {
    path = toLowerString(path);
    for (size_t i = 0; i < _rows.size(); i++) {
        std::string fullPath = _rows[i][REALNAME_COLUMN].text();
        if (toLowerString(fullPath) == path) {
            return i;
        }
    }

    return UINT32_MAX;
}

bool cMainList::getRomInfo(u32 rowIndex, DSRomInfo& info) {
    if (rowIndex < _romInfoList.size()) {
        std::string fullPath = getRowFullPath(rowIndex);
        info = DSRomInfo();
        processDirIcon(info, fullPath);

        return true;
    } else {
        return false;
    }
}

void cMainList::scheduleRomSelection(const std::string& romPath) {
    _scheduledRom = romPath;
}

void cMainList::selectRom(const std::string& romPath) {
    selectRom(romPath, false);
}

void cMainList::selectRom(const std::string& romPath, bool silent) {
    for (size_t row = 0; row < _rows.size(); row++) {
        if (romPath == _rows[row][REALNAME_COLUMN].text()) {
            selectRow(row, silent);
            return;
        }
    }

    if (_rows.size() > 0) {
        selectRow(0, silent);
    }
}

void cMainList::draw() {
    cListView::draw();
    drawIcons();
}

void cMainList::drawBackdrop() {
    drawItemBackgrounds();
}

void cMainList::drawItemBackgrounds() {
    if (_viewMode == VM_LIST || _viewMode == VM_LIST_ICON) {
        return;
    }

    if (!_itemBg->valid()) {
        return;
    }

    size_t total = onScreenRowCount();

    _itemBg->setAlpha(gdi().getMainEngineLayer() == MEL_DOWN);

    for (size_t i = 0; i < total; i++) {
        s32 itemX = 0;
        s32 itemY = i * _rowHeight;

        _itemBg->setRelativePosition(cPoint(itemX, itemY));
        _itemBg->draw();
    }
}

void cMainList::drawIcons() {
    if (_viewMode == VM_LIST) {
        return;
    }

    size_t total = onScreenRowCount();

    bool small = (_viewMode == VM_LIST_ICON) ? true : false;
    int iconHeight = small ? 16 : 32;
    int prefix = small ? 0 : _iconPrefix;

    for (size_t i = 0; i < total; i++) {
        int romId = (_firstVisibleRowId + i) % total;

        s32 itemX = position().x + prefix;
        s32 itemY = position().y + i * _rowHeight + ((_rowHeight - iconHeight) >> 1) - 1;
        
        _romInfoList[romId].drawDSRomIcon(itemX, itemY, small, _engine);
    }
}

void cMainList::setViewMode(VIEW_MODE mode) {
    if (!_columns.size()) return;
    _viewMode = mode;

    switch (_viewMode) {
        case VM_LIST:
            _columns[ICON_COLUMN].width = 0;
            _columns[SHOWNAME_COLUMN].width = size().x;
            _columns[SHOWNAME_COLUMN].mainFont = false;
            _columns[INTERNALNAME_COLUMN].width = 0;
            arangeColumnsSize();
            setRowHeight(15);
            break;
        case VM_LIST_ICON:
            _columns[ICON_COLUMN].width = 21;
            _columns[SHOWNAME_COLUMN].width = size().x - 21;
            _columns[SHOWNAME_COLUMN].mainFont = false;
            _columns[INTERNALNAME_COLUMN].width = 0;
            arangeColumnsSize();
            setRowHeight(18);
            break;
        case VM_ICON:
            _columns[ICON_COLUMN].width = _iconPrefix + 32 + _iconSufix + _textPrefix;
            _columns[SHOWNAME_COLUMN].width = size().x - _iconPrefix - 32 - _iconSufix - _textPrefix - _textSufix;
            _columns[SHOWNAME_COLUMN].mainFont = true;
            _columns[INTERNALNAME_COLUMN].width = 0;
            arangeColumnsSize();
            setRowHeight(_tallRowHeight);
            break;
        case VM_INTERNAL:
            _columns[ICON_COLUMN].width = _iconPrefix + 32 + _iconSufix + _textPrefix;
            _columns[SHOWNAME_COLUMN].width = 0;
            _columns[INTERNALNAME_COLUMN].width = size().x - _iconPrefix - 32 - _iconSufix - _textPrefix - _textSufix;
            _columns[INTERNALNAME_COLUMN].center = _centerInternalColumn;
            _columns[INTERNALNAME_COLUMN].mainFont = true;
            arangeColumnsSize();
            setRowHeight(_tallRowHeight);
            break;
    }

    scrollTo(_selectedRowId - onScreenRowCount() + 1);
}

std::string cMainList::getCurrentDir() {
    return _currentDir;
}
