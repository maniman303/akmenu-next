/*
    mainlist.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

//�

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

using namespace akui;

cMainList::cMainList(cWindow* parent, const std::string& text)
    : cListView(4, 20, 248, 152, parent, text),
      _showAllFiles(false),
      _topCount(5),
      _topuSD(0),
      _topuDSiSD(1),
      _topFavorites(4),
      _topSlot1(2),
      _topSlot2(3) {
    _textOffset = 0;
    _tallRowHeight = 38;
    _centerInternalColumn = false;
    _viewMode = VM_LIST;
    _activeIconScale = 1;
    _activeIcon.hide();
    _activeIcon.update();
    animationManager().addAnimation(&_activeIcon);
    dbg_printf("_activeIcon.init\n");

    if (!isDSiMode()) {
        _topCount = 3;
        _topuSD = 0;
        _topSlot2 = 1;
        _topFavorites = 2;
        _topuDSiSD = 3;
        _topSlot1 = 4;
    } else {
        if (fsManager().isFlashcart()) {
            if (fsManager().isSDInserted()) {
                _topCount = 3;
                _topuSD = 0;
                _topuDSiSD = 1;
                _topFavorites = 2;
                _topSlot2 = 3;               
                _topSlot1 = 4;
            } else {
                _topCount = 2;
                _topuSD = 0;
                _topFavorites = 1;
                _topSlot2 = 2;
                _topuDSiSD = 3;
                _topSlot1 = 4;
            }
        } else {
                _topCount = 3;      
                _topuDSiSD = 0;
                _topFavorites = 1;
                _topSlot1 = 2;
                _topuSD = 3;
                _topSlot2 = 4;
        }
    }
}

cMainList::~cMainList() {
    if (_itemBg) {
        delete _itemBg;
    }
}

int cMainList::init() {
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _textOffset = ini.GetInt("main list", "textOffset", 0);
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

    s32 w = _parent->size().x - (2 * x);
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

    _activeIcon.hide();

    return 1;
}

static bool itemSortComp(const cListView::itemVector& item1, const cListView::itemVector& item2) {
    const std::string& fn1 = item1[cMainList::SHOWNAME_COLUMN].text();
    const std::string& fn2 = item2[cMainList::SHOWNAME_COLUMN].text();

    if (fn1 == "../" || fn1 == "..") return true;
    if (fn2 == "../" || fn2 == "..") return false;

    const std::string& realFn1 = item1[cMainList::REALNAME_COLUMN].text();
    const std::string& realFn2 = item2[cMainList::REALNAME_COLUMN].text();

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

    if (!gs().showHiddenFiles && entryName[0] == '.') {
        return true;
    }

    entryName = toLowerString(entryName);
    if (gs().fileListType == 0 && entryName == "saves") {
        return true;
    }
    
    if (gs().filePresentationMode == 0) {
        return false;
    }

    if (entryName[0] == '_') {
        return true;
    }

    for (size_t i = 0; i < entryNames.size(); ++i) {
        if (entryName == entryNames[i]) {
            return true;
        }
    }

    return false;
}

static std::string getIconPath(std::string iconName) {
    std::string basePath = fsManager().resolveSystemPath("/_nds/akmenunext/");
    std::string uiIconPath = formatString("%sui/%s/icons/%s", basePath.c_str(), gs().uiName.c_str(), iconName.c_str());

    if (fsManager().fileExists(uiIconPath)) {
        return uiIconPath;
    }

    return formatString("%sicons/%s", basePath.c_str(), iconName.c_str());
}

bool cMainList::insertEntryRow(size_t index, const std::vector<std::string>& texts, const DSRomInfo& romInfo) {
    std::vector<std::string> copy(texts);
    if (copy.size() <= IS_FAVORITE_COLUMN) {
        copy.push_back("false");
    }
    
    if (!insertRow(index, copy)) {
        return false;
    }

    _romInfoList.push_back(romInfo);

    return true;
}

void cMainList::processDirIcons() {
    std::string folder = getIconPath("folder_banner.bin");
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
                rominfo.MayBeGbaRom(filename);
            } else if (".nds" != extName && ".dsi" != extName && ".srl" != extName) {
                memcpy(&rominfo.banner(), unknown_banner_bin, sizeof(tNDSBanner));
                allowUnknown = true;
            } else {
                rominfo.MayBeDSRom(filename);
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

bool cMainList::setupDefaultDir(bool skipCards, bool skipFavorites) {
    std::string microsd = getIconPath("microsd_banner.bin");
    std::string nand = getIconPath("nand_banner.bin");
    std::string gba = getIconPath("gba_banner.bin");
    std::string folder = getIconPath("folder_banner.bin");

    for (size_t i = 0; i < _topCount; ++i) {
        std::vector<std::string> a_row;
        a_row.push_back("");  // make a space for icon
        DSRomInfo rominfo;
        if (_topuSD == i) {
            if (skipCards) {
                continue;
            }

            a_row.push_back(LANG("mainlist", "microsd card"));
            a_row.push_back("");
            a_row.push_back("fat:/");

            rominfo.setBannerFromFile("folder", microsd, microsd_banner_bin);
        } else if (_topuDSiSD == i) {
            if (skipCards) {
                continue;
            }

            a_row.push_back("DSi SD");
            a_row.push_back("");
            a_row.push_back("sd:/");
            
            rominfo.setBannerFromFile("folder", microsd, microsd_banner_bin);
        } else if (_topSlot1 == i) {
            a_row.push_back(LANG("mainlist", "slot1 card"));
            a_row.push_back("");
            a_row.push_back("slot1:/");
            
            rominfo.setBannerFromFile("folder", nand, nand_banner_bin);
        } else if (_topSlot2 == i) {
            u8 chk = CGbaLoader::GetGbaHeader();
            if (chk != GBA_HEADER.complement) {
                continue;
            }

            a_row.push_back(LANG("mainlist", "slot2 card"));
            a_row.push_back("");
            a_row.push_back("slot2:/");
            
            rominfo.setBannerFromFile("folder", gba, gba_banner_bin);
        } else if (_topFavorites == i) {
            if (skipFavorites) {
                continue;
            }

            a_row.push_back(LANG("mainlist", "favorites"));
            a_row.push_back("");
            a_row.push_back("favorites:/");
            
            rominfo.setBannerFromFile("folder", folder, folder_banner_bin);
        }
        
        insertEntryRow(getRowCount(), a_row, rominfo);
    }

    _currentDir = "";
    directoryChanged();

    return true;
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

    CIniFile ini(SFN_FAVORITES);
    std::vector<std::string> favoriteItems;
    ini.GetStringVector("main", "list", favoriteItems, '|');
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

    if (_topuDSiSD < _topCount) {
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
        insertEntryRow(getRowCount(), lastPlayedRow, DSRomInfo());
    }

    std::vector<std::vector<std::string>> favoriteRows = getFavoriteRows(true);
    for (size_t i = 0; i < favoriteRows.size(); i++) {
        insertEntryRow(getRowCount(), favoriteRows[i], DSRomInfo());
    }

    int rowsToLoad = std::max(gs().minimalModeRomsCount, 0);
    rowsToLoad = rowsToLoad - favoriteRows.size();
    if (rowsToLoad <= 0) {
        return false;
    }

    std::vector<std::vector<std::string>> rows = getGameRows(rowsToLoad);
    int rowsCount = static_cast<int>(rows.size());
    for (int i = 0; i < std::min(rowsCount, rowsToLoad); i++) {
        insertEntryRow(getRowCount(), rows[i], DSRomInfo());
    }

    return rowsCount <= rowsToLoad;
}

bool cMainList::enterDir(const std::string& dirName) {
    _saves.clear();
    if (startsWithString(dirName, "...") || dirName.empty())  // select RPG or SD card
    {
        removeAllRows();
        _romInfoList.clear();

        if (gs().filePresentationMode < 2) {
            return setupDefaultDir(false, false);
        }

        bool skipSdCards = setupGameDir();

        std::sort(_rows.begin(), _rows.end(), itemSortComp);

        processDirIcons();

        return setupDefaultDir(skipSdCards, true);
    }

    if ("slot2:/" == dirName) {
        _currentDir = "";
        directoryChanged();
        return true;
    }

    if ("slot1:/" == dirName) {
        _currentDir = "";
        directoryChanged();
        return true;
    }

    bool favorites = ("favorites:/" == dirName);
    if (favorites) {
        removeAllRows();
        _romInfoList.clear();

        std::vector<std::vector<std::string>> favoriteRows = getFavoriteRows(false);
        for (size_t i = 0; i < favoriteRows.size(); i++) {
            insertEntryRow(getRowCount(), favoriteRows[i], DSRomInfo());
        }
    } else {
        struct dirent* entry;
        DIR* dir = opendir(dirName.c_str());

        if (dir == NULL) {
            if (fsManager().getFSRoot() == dirName) {
                std::string title = LANG("sd card error", "title");
                std::string sdError = LANG("sd card error", "text");
                cMessageBox::showModal(title, sdError, MB_OK);
            }
            dbg_printf("Unable to open directory<%s>.\n", dirName.c_str());
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
        std::vector<std::string> savNames;
        savNames.push_back(".sav");

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

            if (extnameFilter(savNames, extName)) {
                _saves.push_back(filePath);
            }

            bool showThis = (entry->d_type == DT_DIR) ? (lfn != "." && lfn != "..") : extnameFilter(extNames, extName);
            showThis = showThis && (_showAllFiles || gs().showHiddenFiles || !(FAT_getAttr(filePath.c_str()) & ATTR_HIDDEN));
            if (!showThis) {
                continue;
            }

            u32 row_count = getRowCount();
            std::vector<std::string> a_row;
            a_row.push_back("");   // make a space for icon
            a_row.push_back(lfn);  // show name
            a_row.push_back("");   // make a space for internal name
            a_row.push_back(filePath);  // real name

            if (entry->d_type == DT_DIR) {
                a_row[SHOWNAME_COLUMN] += "/";
                a_row[REALNAME_COLUMN] += "/";
            }

            size_t insertPos(row_count);
            insertEntryRow(insertPos, a_row, DSRomInfo());
        }

        closedir(dir);
    }

    std::sort(_rows.begin(), _rows.end(), itemSortComp);
    std::sort(_saves.begin(), _saves.end(), stringComp);

    processDirIcons();

    _currentDir = dirName;

    directoryChanged();

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

void cMainList::onSelectChanged(u32 index) {
    dbg_printf("%s\n", _rows[index][REALNAME_COLUMN].text().c_str());
}

void cMainList::onScrolled(u32 index) {
    _activeIconScale = 1;
    // updateActiveIcon( CONTENT );
}

void cMainList::backParentDir() {
    if ("..." == _currentDir) return;

    bool fat1 = (fsManager().getFSRoot() == _currentDir);
    bool favorites = ("favorites:/" == _currentDir);
    if (_currentDir == "fat:/" || _currentDir == "sd:/" || fat1 || favorites || _currentDir == "/") {
        enterDir("...");
        if (fat1) {
            selectRow(SDCard());
        } else if (favorites) {
            selectRow(gs().filePresentationMode < 2 ? _topFavorites : 0);
        }

        return;
    }

    size_t pos = _currentDir.rfind("/", _currentDir.size() - 2);
    std::string parentDir = _currentDir.substr(0, pos + 1);
    dbg_printf("%s->%s\n", _currentDir.c_str(), parentDir.c_str());

    std::string oldCurrentDir = _currentDir;

    if (enterDir(parentDir)) {  // select last entered director
        for (size_t i = 0; i < _rows.size(); ++i) {
            if (parentDir + _rows[i][REALNAME_COLUMN].text() == oldCurrentDir) {
                selectRow(i);
            }
        }
    }
}

bool cMainList::processKeyMessage(cKeyMessage message) {
    if (message.isKeyUp(KEY_B)) {
        backParentDir();
        return true;
    }

    return cListView::processKeyMessage(message);
}

std::string cMainList::getSelectedFullPath() {
    if (!_rows.size()) return std::string("");
    return _rows[_selectedRowId][REALNAME_COLUMN].text();
}

std::string cMainList::getSelectedShowName() {
    if (!_rows.size()) return std::string("");
    return _rows[_selectedRowId][SHOWNAME_COLUMN].text();
}

std::string cMainList::getSelectedFileName() {
    if (!_rows.size()) return std::string("");
    std::string fullPath = _rows[_selectedRowId][REALNAME_COLUMN].text();
    size_t lastSlashPos = fullPath.find_last_of("/\\");
    return fullPath.substr(lastSlashPos + 1);
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

void cMainList::selectRom(const std::string& romPath){
    for (size_t row = 0; row < _rows.size(); row++) {
        if (romPath == _rows[row][REALNAME_COLUMN].text()) {
            selectRow(row);
            break;
        }
    }
}

void cMainList::draw() {
    updateInternalNames();
    drawItemBackgrounds();
    cListView::draw();
    updateActiveIcon(POSITION);
    drawIcons();
}

void cMainList::drawItemBackgrounds() {
    if (_viewMode == VM_LIST || _viewMode == VM_LIST_ICON) {
        return;
    }

    size_t total = std::min(_visibleRowCount, _rows.size() - _firstVisibleRowId);

    for (size_t i = 0; i < total; i++) {
        s32 itemX = 0 - relativePosition().x;
        s32 itemY = i * _rowHeight;

        _itemBg->setRelativePosition(cPoint(itemX, itemY));
        _itemBg->draw();
    }
}

void cMainList::drawIcons()
{
    if (_viewMode == VM_LIST) {
        return;
    }

    size_t total = std::min(_visibleRowCount, _rows.size() - _firstVisibleRowId);

    bool small = (_viewMode == VM_LIST_ICON) ? true : false;
    int icon_height = small ? 16 : 32;

    for (size_t i = 0; i < total; ++i) {
        if (_firstVisibleRowId + i == _selectedRowId && _activeIcon.visible()) {
            continue;
        }
        s32 itemX = position().x + 1;
        s32 itemY = position().y + i * _rowHeight + ((_rowHeight - icon_height) >> 1) - 1;
        _romInfoList[_firstVisibleRowId + i].drawDSRomIcon(itemX, itemY, _engine, small);
    }
}

void cMainList::setViewMode(VIEW_MODE mode) {
    if (!_columns.size()) return;
    _viewMode = mode;

    switch (_viewMode) {
        case VM_LIST:
            _columns[ICON_COLUMN].width = 0;
            _columns[SHOWNAME_COLUMN].width = size().x;
            _columns[INTERNALNAME_COLUMN].width = 0;
            arangeColumnsSize();
            setRowHeight(15);
            break;
        case VM_LIST_ICON:
            _columns[ICON_COLUMN].width = 21;
            _columns[SHOWNAME_COLUMN].width = size().x - 21;
            _columns[INTERNALNAME_COLUMN].width = 0;
            arangeColumnsSize();
            setRowHeight(18);
            break;
        case VM_ICON:
            _columns[ICON_COLUMN].width = 36 + _textOffset;
            _columns[SHOWNAME_COLUMN].width = size().x - 36 - _textOffset;
            _columns[INTERNALNAME_COLUMN].width = 0;
            arangeColumnsSize();
            setRowHeight(_tallRowHeight);
            break;
        case VM_INTERNAL:
            _columns[ICON_COLUMN].width = 36 + _textOffset;
            _columns[SHOWNAME_COLUMN].width = 0;
            _columns[INTERNALNAME_COLUMN].width = size().x - 36 - _textOffset;
            _columns[INTERNALNAME_COLUMN].center = _centerInternalColumn;
            arangeColumnsSize();
            setRowHeight(_tallRowHeight);
            break;
    }

    scrollTo(_selectedRowId - _visibleRowCount + 1);
}

void cMainList::updateActiveIcon(bool updateContent) {
    const INPUT& temp = getInput();
    bool allowAnimation = true;
    animateIcons(allowAnimation);

    // do not show active icon when hold key to list files. Otherwise the icon will not show
    // correctly.
    if (getInputIdleMs() > 1000 && VM_LIST != _viewMode && VM_LIST_ICON != _viewMode && allowAnimation &&
        _romInfoList.size() && 0 == temp.keysHeld && gs().Animation) {
        if (!_activeIcon.visible()) {
            u8 backBuffer[32 * 32 * 2];
            zeroMemory(backBuffer, 32 * 32 * 2);
            _romInfoList[_selectedRowId].drawDSRomIconMem(backBuffer);
            memcpy(_activeIcon.buffer(), backBuffer, 32 * 32 * 2);
            _activeIcon.setBufferChanged();

            s32 itemX = position().x;
            s32 itemY = position().y + (_selectedRowId - _firstVisibleRowId) * _rowHeight +
                        ((_rowHeight - 32) >> 1) - 1;
            _activeIcon.setPosition(itemX, itemY);
            _activeIcon.show();
            dbg_printf("sel %d ac ico x %d y %d\n", _selectedRowId, itemX, itemY);
            for (u8 i = 0; i < 8; ++i) dbg_printf("%02x", backBuffer[i]);
            dbg_printf("\n");
        }
    } else if (_activeIcon.visible()) {
        _activeIcon.hide();
        cwl();
    }
}

std::string cMainList::getCurrentDir() {
    return _currentDir;
}

void cMainList::updateInternalNames(void) {
    if (_viewMode == VM_INTERNAL) {
        size_t total = _visibleRowCount;
        if (total > _rows.size() - _firstVisibleRowId) total = _rows.size() - _firstVisibleRowId;
        for (size_t ii = 0; ii < total; ++ii) {
            if (0 == _rows[_firstVisibleRowId + ii][INTERNALNAME_COLUMN].text().length()) {
                if (_romInfoList[_firstVisibleRowId + ii].isDSRom()) {
                    _rows[_firstVisibleRowId + ii][INTERNALNAME_COLUMN].setText(
                            unicode_to_local_string(_romInfoList[_firstVisibleRowId + ii]
                                                            .banner()
                                                            .titles[gs().language],
                                                    128, NULL));
                } else {
                    _rows[_firstVisibleRowId + ii][INTERNALNAME_COLUMN].setText(
                            _rows[_firstVisibleRowId + ii][SHOWNAME_COLUMN].text());
                }
            }
        }
    }
}

bool cMainList::IsFavorites(void) {
    return ("favorites:/" == _currentDir);
}

const std::vector<std::string>* cMainList::Saves(void) {
    return IsFavorites() ? NULL : &_saves;
}

void cMainList::SwitchShowAllFiles(void) {
    _showAllFiles = !_showAllFiles;
    enterDir(getCurrentDir());
}

u32 cMainList::Slot1() {
    std::string slotName = LANG("mainlist", "slot1 card");
    for (size_t i = 0; i < getRowCount(); i++) {
        if (_rows[i][SHOWNAME_COLUMN].text() == slotName) {
            return i;
        }
    }

    return _topSlot1;
}

u32 cMainList::Slot2() {
    std::string slotName = LANG("mainlist", "slot2 card");
    for (size_t i = 0; i < getRowCount(); i++) {
        if (_rows[i][SHOWNAME_COLUMN].text() == slotName) {
            return i;
        }
    }

    return _topSlot2;
}

u32 cMainList::SDCard() {
    std::string slotName = LANG("mainlist", "microsd card");
    for (size_t i = 0; i < getRowCount(); i++) {
        if (_rows[i][SHOWNAME_COLUMN].text() == slotName) {
            return i;
        }
    }

    return _topuSD;
}
