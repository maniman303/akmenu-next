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
#include "savemngr.h"
#include "logger.h"
#include "ui/msgbox.h"
#include "../../share/memtool.h"
#include "favorites.h"

cMainList::cMainList(cWindow* parent, const std::string& text)
    : cListView(4, 20, 248, 152, parent, text) {
    _showAllFiles = false;
    _textPrefix = 0;
    _textSufix = 0;
    _iconPrefix = 1;
    _iconSufix = 3;
    _tallRowHeight = 38;
    _centerInternalColumn = false;
    _viewMode = VM_LIST;
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
    insertColumn(ROMINFO_COLUMN, "romInfo", 0);

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
    if (0 == extNames.size()) return true;

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

bool cMainList::insertEntryRow(const std::vector<std::string>& texts, const DSRomInfo& romInfo) {
    size_t index = getRowCount();
    std::vector<std::string> copy(texts);
    while (copy.size() < IS_FAVORITE_COLUMN) {
        copy.push_back("");
    }

    if (copy.size() <= IS_FAVORITE_COLUMN) {
        copy.push_back("false");
    }

    if (copy.size() <= ROMINFO_COLUMN) {
        copy.push_back(std::to_string(index));
    } else {
        copy[ROMINFO_COLUMN] = std::to_string(index);
    }
    
    if (!insertRow(index, copy)) {
        return false;
    }

    _romInfoList.push_back(romInfo);

    return true;
}

void cMainList::processDirIcons() {
    std::string folder = fsManager().getIconPath("folder_banner.bin");
    for (size_t ii = 0; ii < _rows.size(); ++ii) {
        DSRomInfo& rominfo = _romInfoList[ii];
        std::string filename = _rows[ii][REALNAME_COLUMN].text();

        if (filename == "fat:/" || filename == "sd:/") {
            continue;
        }

        std::string extName = "";
        size_t lastDotPos = filename.find_last_of('.');
        if (filename.npos != lastDotPos) {
            extName = filename.substr(lastDotPos);
        }

        extName = toLowerString(extName);
        if ('/' == filename.back()) {
            rominfo.setBannerFromFile("folder", folder, folder_banner_bin);
        } else {
            bool allowExt = true, allowUnknown = false;
            if (".sav" == extName) {
                memcpy(&rominfo.banner(), nds_save_banner_bin, sizeof(tNDSBanner));
            } else if (".gba" == extName) {
                rominfo.mayBeGbaRom(filename);
            } else if (".nds" != extName && ".dsi" != extName && ".srl" != extName) {
                memcpy(&rominfo.banner(), unknown_banner_bin, sizeof(tNDSBanner));
                allowUnknown = true;
            } else {
                rominfo.mayBeDSRom(filename);
                allowExt = false;
            }

            rominfo.setExtIcon(_rows[ii][SHOWNAME_COLUMN].text());
            
            if (allowExt && extName.length() && !rominfo.isExtIcon()) {
                rominfo.setExtIcon(extName.substr(1));
            }
                
            if (allowUnknown && !rominfo.isExtIcon()) {
                rominfo.setExtIcon("unknown");
            }
        }
    }
}

void cMainList::setupDefaultDir(bool skipCards, bool skipFavorites) {
    std::string microsd = fsManager().getIconPath("microsd_banner.bin");
    std::string nand = fsManager().getIconPath("nand_banner.bin");
    std::string gba = fsManager().getIconPath("gba_banner.bin");
    std::string folder = fsManager().getIconPath("folder_banner.bin");

    if (!skipCards && fsManager().isFlashcart()) {
        DSRomInfo rominfo;
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "microsd card"));
        a_row.push_back("");
        a_row.push_back("fat:/");

        rominfo.setBannerFromFile("folder", microsd, microsd_banner_bin);

        insertEntryRow(a_row, rominfo);
    }

    if (!skipCards && isDSiMode() && fsManager().isSDInserted()) {
        DSRomInfo rominfo;
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back("DSi SD");
        a_row.push_back("");
        a_row.push_back("sd:/");
        
        rominfo.setBannerFromFile("folder", microsd, microsd_banner_bin);

        insertEntryRow(a_row, rominfo);
    }

    if (!skipFavorites && cFavorites::GetFavorites().size() > 0) {
        DSRomInfo rominfo;
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "favorites"));
        a_row.push_back("");
        a_row.push_back("favorites:/");
        
        rominfo.setBannerFromFile("folder", folder, folder_banner_bin);

        insertEntryRow(a_row, rominfo);
    }

    if (isDSiMode() && !fsManager().isFlashcart()) {
        DSRomInfo rominfo;
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "slot1 card"));
        a_row.push_back("");
        a_row.push_back("slot1:/");
        
        rominfo.setBannerFromFile("folder", nand, nand_banner_bin);

        insertEntryRow(a_row, rominfo);
    }

    if (!isDSiMode() && CGbaLoader::GetGbaHeader() == GBA_HEADER.complement) {
        DSRomInfo rominfo;
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "slot2 card"));
        a_row.push_back("");
        a_row.push_back("slot2:/");
        
        rominfo.setBannerFromFile("folder", gba, gba_banner_bin);

        insertEntryRow(a_row, rominfo);
    }
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

    res.push_back("");  // make a space for icon
    res.push_back(showName);  // show name
    res.push_back("");  // make a space for internal name
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

        std::vector<std::string> a_row;
        a_row.push_back("");  // make a space for icon
        a_row.push_back(showName);  // show name
        a_row.push_back("");  // make a space for internal name
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

            std::string showName(lfn);

            std::vector<std::string> a_row;
            a_row.push_back("");  // make a space for icon
            a_row.push_back(showName);  // show name
            a_row.push_back("");  // make a space for internal name
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

bool cMainList::setupGameDir() {
    std::vector<std::string> lastPlayedRow = getLastPlayedRow();
    if (lastPlayedRow.size() > 0) {
        insertEntryRow(lastPlayedRow, DSRomInfo());
    }

    std::vector<std::vector<std::string>> favoriteRows = getFavoriteRows(true);
    for (size_t i = 0; i < favoriteRows.size(); i++) {
        insertEntryRow(favoriteRows[i], DSRomInfo());
    }

    int rowsToLoad = std::max(gs().minimalModeRomsCount, 0);
    rowsToLoad = rowsToLoad - favoriteRows.size();
    if (rowsToLoad <= 0) {
        return false;
    }

    std::vector<std::vector<std::string>> rows = getGameRows(rowsToLoad);
    int rowsCount = static_cast<int>(rows.size());
    for (int i = 0; i < std::min(rowsCount, rowsToLoad); i++) {
        insertEntryRow(rows[i], DSRomInfo());
    }

    if (isDSiMode() && !fsManager().isFlashcart()) {
        std::string nand = fsManager().getIconPath("nand_banner.bin");
        std::vector<std::string> slotRow;
        slotRow.push_back(LANG("mainlist", "slot1 card"));
        slotRow.push_back("");
        slotRow.push_back("slot1:/");
        
        DSRomInfo romInfo;
        romInfo.setBannerFromFile("folder", nand, nand_banner_bin);

        insertEntryRow(slotRow, romInfo);
    }
    
    if (CGbaLoader::GetGbaHeader() == GBA_HEADER.complement) {
        std::string gba = fsManager().getIconPath("gba_banner.bin");
        std::vector<std::string> slotRow;
        slotRow.push_back(LANG("mainlist", "slot2 card"));
        slotRow.push_back("");
        slotRow.push_back("slot2:/");
        
        DSRomInfo romInfo;
        romInfo.setBannerFromFile("folder", gba, gba_banner_bin);

        insertEntryRow(slotRow, romInfo);
    }

    return rowsCount <= rowsToLoad;
}

void cMainList::onDirectoryChanged(bool changed) {
    processDirIcons();
    updateInternalNames();
    std::sort(_rows.begin(), _rows.end(), itemSortComp);
    scheduleBackdrop();
    
    if (changed) {
        directoryChanged();
    }
}

bool cMainList::enterDir(const std::string& dirName) {
    std::string oldDir = _currentDir;
    if (startsWithString(dirName, "...") || dirName.empty())  // select RPG or SD card
    {
        removeAllRows();
        _romInfoList.clear();
        _currentDir = "...";

        if (gs().filePresentationMode < 2) {
            setupDefaultDir(false, false);
            onDirectoryChanged(_currentDir != oldDir);

            return true;
        }

        bool skipSdCards = setupGameDir();
        setupDefaultDir(skipSdCards, true);
        onDirectoryChanged(_currentDir != oldDir);

        return true;
    }

    if ("slot2:/" == dirName) {
        _currentDir = "";
        onDirectoryChanged(true);
        return true;
    }

    if ("slot1:/" == dirName) {
        _currentDir = "";
        onDirectoryChanged(true);
        return true;
    }

    bool favorites = ("favorites:/" == dirName);
    if (favorites) {
        removeAllRows();
        _romInfoList.clear();

        std::vector<std::vector<std::string>> favoriteRows = getFavoriteRows(false);
        for (size_t i = 0; i < favoriteRows.size(); i++) {
            insertEntryRow(favoriteRows[i], DSRomInfo());
        }
    } else {
        struct dirent* entry;
        DIR* dir = opendir(dirName.c_str());

        if (dir == NULL) {
            if (fsManager().getFSRoot() == dirName) {
                std::string title = LANG("sd card error", "title");
                std::string sdError = LANG("sd card error", "text");
                akui::cMessageBox::showModal(title, sdError, MB_OK);
            }

            return false;
        }

        removeAllRows();
        _romInfoList.clear();

        std::vector<std::string> extNames;
        extNames.push_back(".nds");
        extNames.push_back(".dsi");
        extNames.push_back(".srl");
        if (gs().showGbaRoms > 0) extNames.push_back(".gba");
        if (gs().fileListType > 0) extNames.push_back(".sav");
        if (_showAllFiles || gs().fileListType > 1) extNames.clear();

        std::vector<std::string> entryNames;
        entryNames.push_back("boot.nds");

        while ((entry = readdir(dir)) != NULL) {
            std::string lfn(entry->d_name);

            // Don't show system or hidden files and dirs
            if (hiddenEntryFilter(entryNames, lfn)) {
                continue;
            }

            std::string extName = "";
            size_t lastDotPos = lfn.find_last_of('.');
            if (lfn.npos != lastDotPos) {
                extName = lfn.substr(lastDotPos);
            }

            std::string filePath = dirName + lfn;

            bool showThis = (entry->d_type == DT_DIR) ? (lfn != "." && lfn != "..") : extnameFilter(extNames, extName);
            showThis = showThis && (_showAllFiles || !(FAT_getAttr(filePath.c_str()) & ATTR_HIDDEN));
            if (!showThis) {
                continue;
            }

            std::vector<std::string> a_row;
            a_row.push_back("");   // make a space for icon
            a_row.push_back(lfn);  // show name
            a_row.push_back("");   // make a space for internal name
            a_row.push_back(filePath);  // real name

            if (entry->d_type == DT_DIR) {
                a_row[SHOWNAME_COLUMN] += "/";
                a_row[REALNAME_COLUMN] += "/";
            }

            insertEntryRow(a_row, DSRomInfo());
        }

        closedir(dir);
    }

    _currentDir = dirName;

    onDirectoryChanged(_currentDir != oldDir);

    return true;
}

std::string cMainList::processItemText(std::string text, int column) {
    if (column != SHOWNAME_COLUMN) {
        return text;
    }

    text = replaceInString(text, "; ", ": ");

    if (_showAllFiles) {
        return text;
    }

    if (gs().filePresentationMode == 0) {
        return text;
    }

    if (!text.empty() && text.back() == '/') {
        text.pop_back();

        if (text == "saves") {
            return "Saves";
        }

        return text;
    }

    size_t lastdot = text.find_last_of(".");
    if (lastdot == std::string::npos) {
        return text;
    }

    std::string extName;
    if (text.npos != lastdot) {
        extName = text.substr(lastdot);
    } else {
        extName = "";
    }

    extName = toLowerString(extName);
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
            selectRow(slotSDCard(), true);
        } else if (favorites) {
            selectRow(slotFavorites(), true);
        }

        return true;
    }

    size_t pos = _currentDir.rfind("/", _currentDir.size() - 2);
    std::string parentDir = _currentDir.substr(0, pos + 1);
    std::string oldCurrentDir = _currentDir;

    if (enterDir(parentDir)) {  // select last entered director
        for (size_t i = 0; i < _rows.size(); ++i) {
            if (parentDir + _rows[i][REALNAME_COLUMN].text() == oldCurrentDir) {
                selectRow(i);
            }
        }

        return true;
    }

    return false;
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
    u32 visibleRowId = _selectedRowId - _firstVisibleRowId;
    cPoint rowPos = position() + cPoint(0, (visibleRowId * _rowHeight) - 1);
    cSize rowSize = cSize(size().x, _rowHeight + 1);

    return cRect(rowPos, rowSize, false);
}

std::string cMainList::getRowFullPath(u32 id) {
    if (!_rows.size() || id >= (u32)_rows.size()) {
        return std::string("");
    }

    return _rows[id][REALNAME_COLUMN].text();
}

std::string cMainList::getRowShowName(u32 id) {
    if (!_rows.size() || id >= (u32)_rows.size()) {
        return std::string("");
    }

    return _rows[id][SHOWNAME_COLUMN].text();
}

std::string cMainList::getRowFileName(u32 id) {
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

bool cMainList::getRomInfo(u32 rowIndex, DSRomInfo& info) const {
    if (rowIndex < _romInfoList.size()) {
        info = _romInfoList[rowIndex];
        return true;
    } else {
        return false;
    }
}

void cMainList::setRomInfo(u32 rowIndex, const DSRomInfo& info) {
    if (!_romInfoList[rowIndex].isDSRom()) return;

    if (rowIndex < _romInfoList.size()) {
        _romInfoList[rowIndex] = info;
    }
}

void cMainList::selectRom(const std::string& romPath) {
    selectRom(romPath, false);
}

void cMainList::selectRom(const std::string& romPath, bool silent) {
    for (size_t row = 0; row < _rows.size(); row++) {
        if (romPath == _rows[row][REALNAME_COLUMN].text()) {
            selectRow(row, silent);
            break;
        }
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

    for (size_t i = 0; i < total; ++i) {
        itemVector item = _rows[_firstVisibleRowId + i];
        if (item.size() <= ROMINFO_COLUMN) {
            continue;
        }

        int romId = std::stoi(item[ROMINFO_COLUMN].text());

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

void cMainList::updateInternalNames(void) {
    if (_viewMode == VM_INTERNAL) {
        size_t total = _rows.size();
        for (size_t ii = 0; ii < total; ++ii) {
            if (_romInfoList[ii].isDSRom()) {
                _rows[ii][INTERNALNAME_COLUMN].setText(_romInfoList[ii].getDsLocTitle());
            } else {
                _rows[ii][INTERNALNAME_COLUMN].setText(
                        _rows[ii][SHOWNAME_COLUMN].text());
            }
        }
    }
}

bool cMainList::IsFavorites(void) {
    return ("favorites:/" == _currentDir);
}

void cMainList::SwitchShowAllFiles(void) {
    _showAllFiles = !_showAllFiles;
    enterDir(getCurrentDir());
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
