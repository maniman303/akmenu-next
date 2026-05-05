#include <unordered_set>
#include <dirent.h>
#include "directoryload.h"
#include "taskcruncher.h"
#include "../dsrom.h"
#include "../favorites.h"
#include "../fsmngr.h"
#include "../gbaloader.h"
#include "../globalsettings.h"
#include "../language.h"
#include "../logger.h"
#include "../mainlist.h"
#include "../savemngr.h"
#include "../stringtool.h"

static bool itemSortComp(const std::vector<std::string>& item1, const std::vector<std::string>& item2) {
    const std::string& realFn1 = item1[cMainList::REALNAME_COLUMN];
    const std::string& realFn2 = item2[cMainList::REALNAME_COLUMN];

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

    const bool isLastFn1 = (item1.size() > cMainList::IS_FAVORITE_COLUMN) && (item1[cMainList::IS_FAVORITE_COLUMN] == "last");
    const bool isLastFn2 = (item2.size() > cMainList::IS_FAVORITE_COLUMN) && (item2[cMainList::IS_FAVORITE_COLUMN] == "last");

    if (isLastFn1 && !isLastFn2) return true;
    if (!isLastFn1 && isLastFn2) return false;

    const bool isFavFn1 = (item1.size() > cMainList::IS_FAVORITE_COLUMN) && (item1[cMainList::IS_FAVORITE_COLUMN] == "true");
    const bool isFavFn2 = (item2.size() > cMainList::IS_FAVORITE_COLUMN) && (item2[cMainList::IS_FAVORITE_COLUMN] == "true");

    if (isFavFn1 && !isFavFn2) return true;
    if (!isFavFn1 && isFavFn2) return false;

    const std::string& fn1 = gs().viewMode == cGlobalSettings::EViewInternal ? item1[cMainList::INTERNALNAME_COLUMN] : item1[cMainList::SHOWNAME_COLUMN];
    const std::string& fn2 = gs().viewMode == cGlobalSettings::EViewInternal ? item2[cMainList::INTERNALNAME_COLUMN] : item2[cMainList::SHOWNAME_COLUMN];

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

DirectoryLoadTask::DirectoryLoadTask(std::string dirName, std::function<void(std::deque<std::vector<std::string>>&)> onLoadCompleted) {
    _dirName = dirName;
    _onLoadCompleted = onLoadCompleted;
    _onCompleted = [this]() {
        if (_onLoadCompleted) {
            _onLoadCompleted(_data);
        }
    };
}

void DirectoryLoadTask::schedule() {
    taskCruncher().push(this);
}

// TODO:
// 1. Split heavy setups like fav / game scan / path into multiple iterations
// 2. Use special struct instead of stupid and heavy vector
s16 DirectoryLoadTask::process(s16 iter) {
    if (iter == 303) {
        if (_data.size() == 0) {
            std::vector<std::string> a_row;
            a_row.push_back("");   // make a space for icon
            a_row.push_back(LANG("mainlist", "empty"));  // show name
            a_row.push_back(LANG("mainlist", "empty"));  // internal name
            a_row.push_back("...");  // real name

            _data.push_back(a_row);
        }

        std::sort(_data.begin(), _data.end(), itemSortComp);

        return -1;
    }

    if (iter == 0) {
        if (startsWithString(_dirName, "...") || _dirName.empty()) {
            if (gs().filePresentationMode >= 2) {
                _plan.push(STAGE::LAST);
                _plan.push(STAGE::FAV);
                _plan.push(STAGE::SCAN);
            }

            _plan.push(STAGE::DEF);

            return 1;
        }

        if (_dirName == "favorites:/") {
            _plan.push(STAGE::FAV);

            return 1;
        }

        _plan.push(STAGE::PATH);

        return 1;
    }

    if (_plan.empty()) {
        return 303;
    }

    s16 stage = _plan.front();
    switch (stage)
    {
    case STAGE::DEF:
        if (setupDefaultDir()) {
            _plan.pop();
        }
        break;
    case STAGE::SCAN:
        if (setupGameScan()) {
            _plan.pop();
        }
        break;
    case STAGE::FAV:
        if (setupFavorites()) {
            _plan.pop();
        }
        break;
    case STAGE::PATH:
        if (setupPath()) {
            _plan.pop();
        }
        break;
    case STAGE::LAST:
        if (setupLastPlayed()) {
            _plan.pop();
        }
        break;
    default:
        break;
    }

    return 1;
}

void DirectoryLoadTask::setOnCompleted(std::function<void()> onCompleted) { }

const std::unordered_set<std::string>& DirectoryLoadTask::getFavorites() {
    if (_favorites.empty()) {
        std::vector<std::string> favoriteList = cFavorites::GetFavorites();
        _favorites = std::unordered_set<std::string>(favoriteList.begin(), favoriteList.end());
        _favorites.emplace("marker");
    }

    return _favorites;
}

bool DirectoryLoadTask::setupLastPlayed() {
    std::vector<std::string> res;

    std::string lastPlayedPath = saveManager().getLastInfo();
    if (!fsManager().fileExists(lastPlayedPath)) {
        return true;
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
            return true;
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

    _data.push_back(res);

    return true;
}

bool DirectoryLoadTask::setupFavorites() {
    bool exclusive = _data.size() > 0;

    std::unordered_set<std::string> favoriteItems = getFavorites();
    for (const std::string& item : favoriteItems) {
        if (item.empty() || (item.back() == '/' && exclusive)) {
            continue;
        }

        if (item == "marker") {
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

        _data.push_back(a_row);
    }

    return true;
}

bool DirectoryLoadTask::setupDefaultDir() {
    // logger().info("Setup def dir.");

    bool skipCards = gs().filePresentationMode >= 2 && _data.size() < (size_t)gs().minimalModeRomsCount;
    bool skipFavorites = gs().filePresentationMode >= 2;

    std::string microsd = fsManager().getIconPath("microsd_banner.bin");
    std::string nand = fsManager().getIconPath("nand_banner.bin");
    std::string gba = fsManager().getIconPath("gba_banner.bin");
    std::string folder = fsManager().getIconPath("folder_banner.bin");

    if (!skipCards && fsManager().isFlashcart()) {
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "microsd card"));
        a_row.push_back(LANG("mainlist", "microsd card"));
        a_row.push_back("fat:/");

        _data.push_back(a_row);
    }

    if (!skipCards && isDSiMode() && fsManager().isSDInserted()) {
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back("DSi SD");
        a_row.push_back("DSi SD");
        a_row.push_back("sd:/");

        _data.push_back(a_row);
    }

    if (!skipFavorites && cFavorites::GetFavorites().size() > 0) {
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "favorites"));
        a_row.push_back(LANG("mainlist", "favorites"));
        a_row.push_back("favorites:/");

        _data.push_back(a_row);
    }

    if (isDSiMode() && !fsManager().isFlashcart()) {
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "slot1 card"));
        a_row.push_back(LANG("mainlist", "slot1 card"));
        a_row.push_back("slot1:/");

        _data.push_back(a_row);
    }

    if (!isDSiMode() && CGbaLoader::GetGbaHeader() == GBA_HEADER.complement) {
        std::vector<std::string> a_row;
        a_row.push_back("");
        a_row.push_back(LANG("mainlist", "slot2 card"));
        a_row.push_back(LANG("mainlist", "slot2 card"));
        a_row.push_back("slot2:/");

        _data.push_back(a_row);
    }

    return true;
}

bool DirectoryLoadTask::setupGameScan() {
    s16 rowsToLoad = gs().minimalModeRomsCount;
    rowsToLoad = rowsToLoad - _data.size();
    if (rowsToLoad <= 0) {
        return true;
    }

    std::unordered_set<std::string> favoriteItems = getFavorites();
    std::queue<std::string> paths;
    paths.push("fat:/");

    if (isDSiMode() && fsManager().isSDInserted()) {
        paths.push("sd:/");
    }

    s16 rows = 0;
    while (rows <= rowsToLoad && !paths.empty()) {
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

            rows++;
            _data.push_back(a_row);
        }

        closedir(dir);
    }

    return true;
}

bool DirectoryLoadTask::setupPath() {
    struct dirent* entry;
    DIR* dir = opendir(_dirName.c_str());

    if (dir == NULL) {
        return true;
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
            continue;
        }

        std::string extName = "";
        size_t lastDotPos = lfn.find_last_of('.');
        if (lfn.npos != lastDotPos) {
            extName = toLowerString(lfn.substr(lastDotPos));
        }

        std::string filePath = _dirName + lfn;

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
            a_row[cMainList::REALNAME_COLUMN] += "/";
        }

        _data.push_back(a_row);
    }

    closedir(dir);

    return true;
}