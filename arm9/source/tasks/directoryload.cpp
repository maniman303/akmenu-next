#include <unordered_set>
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

#define MIN_ROWS 1
#define MAX_ROWS 2

static bool itemSortComp(const std::vector<std::string>& item1, const std::vector<std::string>& item2) {
    const std::string& realFn1 = item1[3];
    const std::string& realFn2 = item2[3];

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

    const bool isLastFn1 = (item1.size() > 4) && (item1[4] == "last");
    const bool isLastFn2 = (item2.size() > 4) && (item2[4] == "last");

    if (isLastFn1 && !isLastFn2) return true;
    if (!isLastFn1 && isLastFn2) return false;

    const bool isFavFn1 = (item1.size() > 4) && (item1[4] == "true");
    const bool isFavFn2 = (item2.size() > 4) && (item2[4] == "true");

    if (isFavFn1 && !isFavFn2) return true;
    if (!isFavFn1 && isFavFn2) return false;

    const std::string& fn1 = gs().viewMode == cGlobalSettings::EViewInternal ? item1[2] : item1[1];
    const std::string& fn2 = gs().viewMode == cGlobalSettings::EViewInternal ? item2[2] : item2[1];

    return fn1 < fn2;
}

static bool extnameFilter(const std::unordered_set<std::string>& extNames, const std::string& extName) {
    if (extNames.size() == 0) return true;

    if (extNames.find(extName) != extNames.end()) {
        return true;
    }

    return false;
}

static bool hiddenEntryFilter(const std::string& entryName) {
    if (entryName.empty()) {
        return true;
    }

    if (gs().filePresentationMode == 0 && entryName != "." && entryName != "..") {
        return false;
    }

    if (entryName[0] == '.') {
        return true;
    }

    if (entryName[0] == '_') {
        return true;
    }

    if (strcasecmp(entryName.c_str(), "boot.nds") == 0) {
        return true;
    }

    if (gs().fileListType == 0 && strcasecmp(entryName.c_str(), "saves") == 0) {
        return true;
    }

    return false;
}

DirectoryLoadTask::DirectoryLoadTask(std::vector<WorkIndicator*> indicators, std::string dirName, std::function<void(std::deque<std::vector<std::string>>&)> onLoadCompleted) {
    _indicators = indicators;
    _dirName = dirName;
    _onLoadCompleted = onLoadCompleted;
    _favoritesIter = _favorites.end();
    _scanDir = NULL;
    _pathDir = NULL;
    _extraOnCompleted = {};
    _onCompleted = [this]() {
        if (_onLoadCompleted) {
            _onLoadCompleted(_data);
        }

        if (_extraOnCompleted) {
            _extraOnCompleted();
        }
    };
}

DirectoryLoadTask::~DirectoryLoadTask() {
    if (_scanDir != NULL) {
        closedir(_scanDir);
    }

    if (_pathDir != NULL) {
        closedir(_pathDir);
    }
}

void DirectoryLoadTask::schedule() {
    taskCruncher().push(this);
}

s16 DirectoryLoadTask::process(s16 iter) {
    if (iter == 303) {
        if (isBusy()) {
            return iter;
        }

        if (_data.size() == 0) {
            std::string name = LANG("mainlist", "empty");
            _data.push_back({"", name, name, "..."});
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
    case (s16)STAGE::DEF:
        if (setupDefaultDir()) {
            _plan.pop();
        }
        break;
    case (s16)STAGE::SCAN:
        if (setupGameScan()) {
            _plan.pop();
        }
        break;
    case (s16)STAGE::FAV:
        if (setupFavorites()) {
            _plan.pop();
        }
        break;
    case (s16)STAGE::PATH:
        if (setupPath()) {
            _plan.pop();
        }
        break;
    case (s16)STAGE::LAST:
        if (setupLastPlayed()) {
            _plan.pop();
        }
        break;
    default:
        logger().error("Stage default.");
        break;
    }

    return _plan.empty() ? 303 : 1;
}

void DirectoryLoadTask::setOnCompleted(std::function<void()> onCompleted) {
    _extraOnCompleted = onCompleted;
}

bool DirectoryLoadTask::isBusy() {
    for (const WorkIndicator* indicator : _indicators) {
        if (indicator->busy()) {
            return true;
        }
    }

    return false;
}

std::unordered_set<std::string>& DirectoryLoadTask::getFavorites() {
    if (_favorites.empty()) {
        std::vector<std::string> favoriteList = cFavorites::GetFavorites();
        _favorites = std::unordered_set<std::string>(favoriteList.begin(), favoriteList.end());
        _favorites.emplace("marker");
        _favoritesIter = _favorites.end();
    }

    return _favorites;
}

bool DirectoryLoadTask::setupLastPlayed() {
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

    _data.push_back({"", showName, internalName, lastPlayedPath, "last"});

    return true;
}

bool DirectoryLoadTask::setupFavorites() {
    bool exclusive = _data.size() > 0;

    std::unordered_set<std::string>& favorites = getFavorites();
    if (_favoritesIter == favorites.end()) {
        _favoritesIter = favorites.begin();
    }

    s16 iter = 0;
    s16 total = isBusy() ? MIN_ROWS : MAX_ROWS;
    while (_favoritesIter != favorites.end() && iter < total) {
        const std::string& item = *(_favoritesIter++);

        if (item.empty() || (item.back() == '/' && exclusive)) {
            continue;
        }

        if (item == "marker") {
            continue;
        }

        if (exclusive && item == saveManager().getLastInfo()) {
            continue;
        }
        
        iter++;
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

        _data.push_back({"", showName, internalName, item, "true"});
    }

    return _favoritesIter == favorites.end();
}

bool DirectoryLoadTask::setupDefaultDir() {
    bool skipCards = gs().filePresentationMode >= 2 && _data.size() < (size_t)gs().minimalModeRomsCount;
    bool skipFavorites = gs().filePresentationMode >= 2;

    std::string microsd = fsManager().getIconPath("microsd_banner.bin");
    std::string nand = fsManager().getIconPath("nand_banner.bin");
    std::string gba = fsManager().getIconPath("gba_banner.bin");
    std::string folder = fsManager().getIconPath("folder_banner.bin");

    if (!skipCards && fsManager().isFlashcart()) {
        std::string name = LANG("mainlist", "microsd card");
        _data.push_back({"", name, name, "fat:/"});
    }

    if (!skipCards && isDSiMode() && fsManager().isSDInserted()) {
        std::string name = "DSi SD";
        _data.push_back({"", name, name, "sd:/"});
    }

    if (!skipFavorites && cFavorites::GetFavorites().size() > 0) {
        std::string name = LANG("mainlist", "favorites");
        _data.push_back({"", name, name, "favorites:/"});
    }

    if (isDSiMode() && !fsManager().isFlashcart()) {
        std::string name = LANG("mainlist", "slot1 card");
        _data.push_back({"", name, name, "slot1:/"});
    }

    if (!isDSiMode() && CGbaLoader::GetGbaHeader() == GBA_HEADER.complement) {
        std::string name = LANG("mainlist", "slot2 card");
        _data.push_back({"", name, name, "slot2:/"});
    }

    return true;
}

bool DirectoryLoadTask::setupGameScan() {
    s16 rowsToLoad = gs().minimalModeRomsCount - _data.size();
    if (rowsToLoad <= 0) {
        return true;
    }

    std::unordered_set<std::string>& favoriteItems = getFavorites();
    
    if (_pathsToScan.empty()) {
        _pathsToScan.push("fat:/");

        if (isDSiMode() && fsManager().isSDInserted()) {
            _pathsToScan.push("sd:/");
        }
    }

    s16 rows = 0;
    s16 total = isBusy() ? MIN_ROWS : MAX_ROWS;
    while (rows < rowsToLoad && rows < total && !_pathsToScan.empty()) {
        std::string path = _pathsToScan.front();

        struct dirent* entry = NULL;
        if (_scanDir == NULL) {
            _scanDir = opendir(path.c_str());
            if (_scanDir == NULL) {
                _pathsToScan.pop();
                continue;
            }
        }

        while (rows < total && rows < rowsToLoad && (entry = readdir(_scanDir)) != NULL) {
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
                _pathsToScan.push(newPath);
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

            rows++;
            _data.push_back({"", lfn, internalName, fullFilePath});
        }

        if (entry == NULL || rows >= rowsToLoad) {
            closedir(_scanDir);
            _scanDir = NULL;
            _pathsToScan.pop();
        }
    }

    return rows >= rowsToLoad || _pathsToScan.empty();
}

bool DirectoryLoadTask::setupPath() {
    struct dirent* entry = NULL;
    if (_pathDir == NULL) {
        _pathDir = opendir(_dirName.c_str());;
        if (_pathDir == NULL) {
            return true;
        }
    }

    if (gs().fileListType <= 1 && _extNames.size() == 0) {
        _extNames.emplace(".nds");
        _extNames.emplace(".dsi");
        _extNames.emplace(".sri");
        if (gs().showGbaRoms > 0) _extNames.emplace(".gba");
        if (gs().fileListType > 0) _extNames.emplace(".sav");
    } 

    u16 rows = 0;
    s16 total = isBusy() ? MIN_ROWS : MAX_ROWS;
    while (rows < total * 2 && (entry = readdir(_pathDir)) != NULL) {
        std::string lfn(entry->d_name);

        // Don't show system or hidden files and dirs
        if (hiddenEntryFilter(lfn)) {
            continue;
        }

        std::string extName = "";
        size_t lastDotPos = lfn.find_last_of('.');
        if (lfn.npos != lastDotPos) {
            extName = toLowerString(lfn.substr(lastDotPos));
        }

        std::string filePath = _dirName + lfn;

        rows += 2;
        bool showThis = (entry->d_type == DT_DIR || extnameFilter(_extNames, extName)) && (entry->d_type != DT_DIR || !(FAT_getAttr(filePath.c_str()) & ATTR_HIDDEN)); 
        if (!showThis) {
            rows--;
            continue;
        }

        std::string internalName = lfn;
        if (entry->d_type == DT_DIR) {
            rows--;
            filePath += "/";
        } else if (extName == ".nds") {
            DSRomInfo romInfo;
            romInfo.mayBeDSRom(filePath);
            if (romInfo.isDSRom()) {
                internalName = romInfo.getDsLocTitle();
            }
        }

        _data.push_back({"", lfn, internalName, filePath});
    }

    if (entry == NULL) {
        closedir(_pathDir);
        _pathDir = NULL;
        return true;
    }

    return false;
}