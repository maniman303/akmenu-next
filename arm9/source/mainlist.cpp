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

static bool itemSortComp(const akui::cListView::itemVector& item1, const akui::cListView::itemVector& item2) {
    const std::string& realFn1 = item1[cMainList::REALNAME_COLUMN].text();
    const std::string& realFn2 = item2[cMainList::REALNAME_COLUMN].text();

    if (realFn1 == "slot1:/") return true;
    if (realFn2 == "slot1:/") return false;

    if (realFn1 == "slot2:/" && realFn2 != "slot1:/") return true;
    if (realFn1 != "slot1:/" && realFn2 == "slot2:/") return false;

    if (gs().filePresentationMode >= 2) {
        if (realFn1 == "fat:/" && realFn2.back() != '/') return false;
        if (realFn2 == "fat:/" && realFn1.back() != '/') return true;

        if (realFn1 == "sd:/" && realFn2.back() != '/') return false;
        if (realFn2 == "sd:/" && realFn1.back() != '/') return true;
    }

    if (realFn1 == "fat:/") return true;
    if (realFn2 == "fat:/") return false;

    if (realFn1 == "sd:/") return true;
    if (realFn2 == "sd:/") return false;

    if (realFn1.back() == '/' && realFn2.back() != '/') return true;
    if (realFn1.back() != '/' && realFn2.back() == '/') return false;

    const bool isLastFn1 = (item1.size() > cMainList::IS_FAVORITE_COLUMN) && (item1[cMainList::IS_FAVORITE_COLUMN].text() == "last");
    const bool isLastFn2 = (item2.size() > cMainList::IS_FAVORITE_COLUMN) && (item2[cMainList::IS_FAVORITE_COLUMN].text() == "last");

    if (isLastFn1 && !isLastFn2) return true;
    if (!isLastFn1 && isLastFn2) return false;

    const bool isFavFn1 = (item1.size() > cMainList::IS_FAVORITE_COLUMN) && (item1[cMainList::IS_FAVORITE_COLUMN].text() == "true");
    const bool isFavFn2 = (item2.size() > cMainList::IS_FAVORITE_COLUMN) && (item2[cMainList::IS_FAVORITE_COLUMN].text() == "true");

    if (isFavFn1 && !isFavFn2) return true;
    if (!isFavFn1 && isFavFn2) return false;

    const std::string& fn1 = gs().viewMode == cGlobalSettings::EViewInternal ? item1[cMainList::INTERNALNAME_COLUMN].text() : item1[cMainList::SHOWNAME_COLUMN].text();
    const std::string& fn2 = gs().viewMode == cGlobalSettings::EViewInternal ? item2[cMainList::INTERNALNAME_COLUMN].text() : item2[cMainList::SHOWNAME_COLUMN].text();

    return fn1 < fn2;
}

static bool extnameFilter(const std::vector<std::string>& extNames, std::string extName) {
    if (extNames.size() == 0) return true;

    extName = toLowerString(extName);
    for (size_t i = 0; i < extNames.size(); ++i) {
        if (extName == extNames[i]) {
            return true;
        }
    }
    return false;
}

static bool hiddenEntryFilter(const std::vector<std::string>& entryNames, std::string entryName) {
    if (entryName.empty()) {
        return true;
    }

    if (gs().filePresentationMode == 0) {
        return false;
    }

    if (entryName[0] == '.') {
        return true;
    }

    entryName = toLowerString(entryName);
    if (gs().fileListType == 0 && entryName == "saves") {
        return true;
    }

    if (entryName[0] == '_') {
        return true;
    }

    for (size_t i = 0; i < entryNames.size(); i++) {
        if (entryName == entryNames[i]) {
            return true;
        }
    }

    return false;
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
    _romInfoList.clear();
    _romInfoList.resize(_visibleRowCount);

    for (size_t i = 0; i < _visibleRowCount; i++) {
        DSRomInfo& romInfo = _romInfoList[(_firstVisibleRowId + i) % _visibleRowCount];
        std::string fileName = _rows[_firstVisibleRowId + i][REALNAME_COLUMN].text();
        
        processDirIcon(romInfo, fileName);
    }
}

void cMainList::validateDirIcons() {
    for (size_t i = 0; i < _visibleRowCount; i++) {
        DSRomInfo& romInfo = _romInfoList[(_firstVisibleRowId + i) % _visibleRowCount];
        std::string fileName = _rows[_firstVisibleRowId + i][REALNAME_COLUMN].text();
        
        if (romInfo.fileName() == fileName) {
            continue;
        }

        _romInfoList[(_firstVisibleRowId + i) % _visibleRowCount] = DSRomInfo();
        romInfo = _romInfoList[(_firstVisibleRowId + i) % _visibleRowCount];
        processDirIcon(romInfo, fileName);
    }
}

void cMainList::onScrolled(u32 index) {
    validateDirIcons();
}

std::vector<std::vector<std::string>> cMainList::setupDefaultDir(bool skipCards, bool skipFavorites) {
    std::string microsd = fsManager().getIconPath("microsd_banner.bin");
    std::string nand = fsManager().getIconPath("nand_banner.bin");
    std::string gba = fsManager().getIconPath("gba_banner.bin");
    std::string folder = fsManager().getIconPath("folder_banner.bin");

    std::vector<std::vector<std::string>> res;

    if (!skipCards && fsManager().isFlashcart()) {
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "microsd card"));
        a_row.push_back(LANG("mainlist", "microsd card"));
        a_row.push_back("fat:/");

        res.push_back(a_row);
    }

    if (!skipCards && isDSiMode() && fsManager().isSDInserted()) {
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back("DSi SD");
        a_row.push_back("DSi SD");
        a_row.push_back("sd:/");

        res.push_back(a_row);
    }

    if (!skipFavorites && cFavorites::GetFavorites().size() > 0) {
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "favorites"));
        a_row.push_back(LANG("mainlist", "favorites"));
        a_row.push_back("favorites:/");

        res.push_back(a_row);
    }

    if (isDSiMode() && !fsManager().isFlashcart()) {
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "slot1 card"));
        a_row.push_back(LANG("mainlist", "slot1 card"));
        a_row.push_back("slot1:/");

        res.push_back(a_row);
    }

    if (!isDSiMode() && CGbaLoader::GetGbaHeader() == GBA_HEADER.complement) {
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "slot2 card"));
        a_row.push_back(LANG("mainlist", "slot2 card"));
        a_row.push_back("slot2:/");

        res.push_back(a_row);
    }

    return res;
}

std::vector<std::string> cMainList::getLastPlayedRow() {
    std::vector<std::string> res;

    std::string lastPlayedPath = saveManager().getLastInfo();
    if (!fsManager().fileExists(lastPlayedPath)) {
        return res;
    }

    std::string showName(lastPlayedPath);
    size_t pos = showName.rfind('/', showName.length() - 2);
    if (pos != showName.npos) {
        showName = showName.substr(pos + 1, showName.npos);
    }

    std::string extName = "";
    size_t lastDotPos = showName.find_last_of('.');
    if (showName.npos != lastDotPos) {
        extName = toLowerString(showName.substr(lastDotPos));
    }

    std::string internalName = showName;
    if (extName == ".nds") {
        DSRomInfo romInfo;
        romInfo.mayBeDSRom(lastPlayedPath);
        if (!romInfo.isDSRom()) {
            return res;
        }

        internalName = romInfo.getDsLocTitle();
    }

    res.push_back("");  // make a space for icon
    res.push_back(showName);  // show name
    res.push_back(internalName);  // internal name
    res.push_back(lastPlayedPath);  // real name
    res.push_back(""); // space for save type
    res.push_back(""); // space for file size
    res.push_back("last"); // is last played

    return res;
}

std::vector<std::vector<std::string>> cMainList::getFavoriteRows(bool exclusive) {
    std::vector<std::vector<std::string>> favoriteRows;

    std::vector<std::string> favoriteItems = cFavorites::GetFavorites();
    for (size_t i = 0; i < favoriteItems.size(); i++) {
        std::string item = favoriteItems[i];

        if (item.empty() || (item.back() == '/' && exclusive)) {
            continue;
        }

        if (exclusive && item == saveManager().getLastInfo()) {
            continue;
        }
        
        std::string showName(item);
        size_t pos = showName.rfind('/', showName.length() - 2);
        if (pos != showName.npos) {
            showName = showName.substr(pos + 1, showName.npos);
        }

        std::string extName = "";
        size_t lastDotPos = showName.find_last_of('.');
        if (showName.npos != lastDotPos) {
            extName = toLowerString(showName.substr(lastDotPos));
        }

        std::string internalName = showName;
        if (extName == ".nds") {
            DSRomInfo romInfo;
            romInfo.mayBeDSRom(item);
            if (!romInfo.isDSRom()) {
                continue;
            }

            internalName = romInfo.getDsLocTitle();
        }

        std::vector<std::string> a_row;
        a_row.push_back("");  // make a space for icon
        a_row.push_back(showName);  // show name
        a_row.push_back(internalName);  // internal name
        a_row.push_back(item);  // real name
        a_row.push_back(""); // space for save type
        a_row.push_back(""); // space for file size
        a_row.push_back("true"); // is favorite

        favoriteRows.push_back(a_row);
    }

    return favoriteRows;
}

std::vector<std::vector<std::string>> cMainList::getGameRows(int rowsToLoad) {
    std::vector<std::vector<std::string>> rows;

    CIniFile ini(SFN_FAVORITES);
    std::vector<std::string> favoriteList;
    ini.GetStringVector("main", "list", favoriteList, '|');
    std::unordered_set<std::string> favoriteItems(favoriteList.begin(), favoriteList.end());

    std::queue<std::string> paths;
    paths.push("fat:/");

    if (isDSiMode() && fsManager().isSDInserted()) {
        paths.push("sd:/");
    }

    while (static_cast<int>(rows.size()) <= rowsToLoad && !paths.empty()) {
        std::string path = paths.front();
        paths.pop();

        struct dirent* entry;
        DIR* dir = opendir(path.c_str());
        if (dir == NULL) {
            continue;
        }

        while ((entry = readdir(dir)) != NULL) {
            std::string lfn(entry->d_name);

            if (lfn.empty() || lfn[0] == '.' || lfn[0] == '_') {
                continue;
            }

            std::string llfn = toLowerString(lfn);

            if (entry->d_type == DT_DIR) {
                if (llfn == "saves") {
                    continue;
                }

                std::string newPath = formatString("%s%s/", path.c_str(), lfn.c_str());
                paths.push(newPath);
                continue;
            }

            if (entry->d_type != DT_REG) {
                continue;
            }

            std::string extName = "";
            size_t lastDotPos = lfn.find_last_of('.');
            if (lfn.npos != lastDotPos) {
                extName = lfn.substr(lastDotPos);
            }

            extName = toLowerString(extName);
            if (extName != ".nds" || llfn == "boot.nds") {
                continue;
            }

            std::string fullFilePath = formatString("%s%s", path.c_str(), lfn.c_str());
            if (fullFilePath == saveManager().getLastInfo()) {
                continue;
            }

            if (favoriteItems.find(fullFilePath) != favoriteItems.end()) {
                continue;
            }

            std::string internalName = lfn;
            if (extName == ".nds") {
                DSRomInfo romInfo;
                romInfo.mayBeDSRom(fullFilePath);
                if (!romInfo.isDSRom()) {
                    continue;
                }

                internalName = romInfo.getDsLocTitle();
            }

            std::vector<std::string> a_row;
            a_row.push_back("");  // make a space for icon
            a_row.push_back(lfn);  // show name
            a_row.push_back(internalName);  // internal name
            a_row.push_back(fullFilePath);  // real name
            a_row.push_back(""); // space for save type
            a_row.push_back(""); // space for file size
            a_row.push_back("false"); // is favorite

            rows.push_back(a_row);
        }

        closedir(dir);
    }

    return rows;
}

std::vector<std::vector<std::string>> cMainList::setupGameDir() {
    std::vector<std::vector<std::string>> res;
    std::vector<std::string> lastPlayedRow = getLastPlayedRow();
    if (lastPlayedRow.size() > 0) {
        res.push_back(lastPlayedRow);
    }

    std::vector<std::vector<std::string>> favoriteRows = getFavoriteRows(true);
    for (size_t i = 0; i < favoriteRows.size(); i++) {
        res.push_back(favoriteRows[i]);
    }

    int rowsToLoad = gs().minimalModeRomsCount;
    rowsToLoad = rowsToLoad - favoriteRows.size();
    if (rowsToLoad <= 0) {
        return res;
    }

    std::vector<std::vector<std::string>> rows = getGameRows(rowsToLoad);
    int rowsCount = static_cast<int>(rows.size());
    for (int i = 0; i < std::min(rowsCount, rowsToLoad); i++) {
        res.push_back(rows[i]);
    }

    return res;
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
        std::vector<std::string> row = rows.front();
        rows.pop_front();
        insertEntryRow(row);
    }

    processDirIcons();
    scheduleBackdrop();
    
    if (changed) {
        directoryChanged();
    }
}

std::vector<std::vector<std::string>> cMainList::prepareDir(const std::string& dirName) {
    std::vector<std::vector<std::string>> res;
    if (startsWithString(dirName, "...") || dirName.empty())  // select RPG or SD card
    {
        bool skipSdCards = false;
        if (gs().filePresentationMode >= 2) {
            std::vector<std::vector<std::string>> gameEntries = setupGameDir();
            for (std::vector<std::string>& entry : gameEntries) {
                res.push_back(entry);
            }

            skipSdCards = gameEntries.size() < (size_t)gs().minimalModeRomsCount;
        }

        std::vector<std::vector<std::string>> defaultEntries = setupDefaultDir(skipSdCards, gs().filePresentationMode >= 2);
        for (std::vector<std::string>& entry : defaultEntries) {
            res.push_back(entry);
        }

        std::sort(_rows.begin(), _rows.end(), itemSortComp);

        return res;
    }

    bool favorites = (dirName == "favorites:/");
    if (favorites) {
        std::vector<std::vector<std::string>> favoriteRows = getFavoriteRows(false);
        for (size_t i = 0; i < favoriteRows.size(); i++) {
            res.push_back(favoriteRows[i]);
        }
    } else {
        struct dirent* entry;
        DIR* dir = opendir(dirName.c_str());

        if (dir == NULL) {
            return res;
        }

        std::vector<std::string> extNames;
        extNames.push_back(".nds");
        extNames.push_back(".dsi");
        extNames.push_back(".srl");
        if (gs().showGbaRoms > 0) extNames.push_back(".gba");
        if (gs().fileListType > 0) extNames.push_back(".sav");
        if (gs().fileListType > 1) extNames.clear();

        std::vector<std::string> entryNames;
        entryNames.push_back("boot.nds");

        while ((entry = readdir(dir)) != NULL) {
            std::string lfn(entry->d_name);

            // Don't show system or hidden files and dirs
            if (hiddenEntryFilter(entryNames, lfn)) {
                // logger().info("Hiding: " + lfn);
                continue;
            }

            std::string extName = "";
            size_t lastDotPos = lfn.find_last_of('.');
            if (lfn.npos != lastDotPos) {
                extName = toLowerString(lfn.substr(lastDotPos));
            }

            std::string filePath = dirName + lfn;

            bool showThis = (entry->d_type == DT_DIR || extnameFilter(extNames, extName)) && !(FAT_getAttr(filePath.c_str()) & ATTR_HIDDEN); 
            if (!showThis) {
                continue;
            }

            std::string internalName = lfn;
            if (extName == ".nds") {
                DSRomInfo romInfo;
                romInfo.mayBeDSRom(filePath);
                if (!romInfo.isDSRom()) {
                    continue;
                }

                internalName = romInfo.getDsLocTitle();
            }

            std::vector<std::string> a_row;
            a_row.push_back("");   // make a space for icon
            a_row.push_back(lfn);  // show name
            a_row.push_back(internalName);  // internal name
            a_row.push_back(filePath);  // real name

            if (entry->d_type == DT_DIR) {
                a_row[REALNAME_COLUMN] += "/";
            }

            res.push_back(a_row);
        }

        closedir(dir);
    }

    if (res.size() == 0) {
        std::vector<std::string> a_row;
        a_row.push_back("");   // make a space for icon
        a_row.push_back(LANG("mainlist", "empty"));  // show name
        a_row.push_back(LANG("mainlist", "empty"));  // internal name
        a_row.push_back("...");  // real name

        res.push_back(a_row);
    }

    std::sort(_rows.begin(), _rows.end(), itemSortComp);

    return res;
}

bool cMainList::enterDir(const std::string& dirName) {
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
    }

    if (_parent != NULL) {
        _parent->disableInput();
    }

    _busy = true;

    DirectoryLoadTask* task = new DirectoryLoadTask(tempDirName, [this, tempDirName](std::deque<std::vector<std::string>> rows) {
        onDirectoryChanged(rows, tempDirName);
    });

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
    logger().info("Looking for rom: " + romPath);

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

    size_t total = std::min(_visibleRowCount, _rows.size() - _firstVisibleRowId);

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

    size_t total = std::min(_visibleRowCount, _rows.size() - _firstVisibleRowId);

    bool small = (_viewMode == VM_LIST_ICON) ? true : false;
    int iconHeight = small ? 16 : 32;
    int prefix = small ? 0 : _iconPrefix;

    for (size_t i = 0; i < total; i++) {
        int romId = (_firstVisibleRowId + i) % _visibleRowCount;

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

    scrollTo(_selectedRowId - _visibleRowCount + 1);
}

std::string cMainList::getCurrentDir() {
    return _currentDir;
}

u32 cMainList::slotSDCard() {
    std::string slotName = LANG("mainlist", "microsd card");
    for (size_t i = 0; i < getRowCount(); i++) {
        if (_rows[i][SHOWNAME_COLUMN].text() == slotName) {
            return i;
        }
    }

    return 0;
}

u32 cMainList::slotFavorites() {
    std::string slotName = LANG("mainlist", "favorites");
    for (size_t i = 0; i < getRowCount(); i++) {
        if (_rows[i][SHOWNAME_COLUMN].text() == slotName) {
            return i;
        }
    }

    return 0;
}
