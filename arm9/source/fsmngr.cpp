#include <sys/stat.h>
#include <dirent.h>
#include <fat.h>
#include <algorithm>
#include "fsmngr.h"
#include "fifotool.h"
#include "systemfilenames.h"
#include "globalsettings.h"

cFSManager::cFSManager() : _isSDInserted(false), _isFlashcart(false), _fsRoot() {

}

void cFSManager::init(int argc, char* argv[]) {
    _isSDInserted = checkSDInserted();

    // We're always on a flashcart if we're running in NTR mode
    if (!isDSiMode()) {
        _isFlashcart = true;
    // If no SD is inserted, we can only be on a flashcart
    } else if (!isSDInserted()) {
        _isFlashcart = true;
    // If argv tells us we've launched from fat, we're on a flashcart
    }else if (argc > 0 && strncmp(argv[0], "fat:/", 5) == 0) {
        _isFlashcart = true;
    }

    // Mount devices
    if (isFlashcart()) {
        fatMountSimple("fat", dldiGetInternal());
        chdir("fat:/");
        _fsRoot = "fat:";
    }

    if (isSDInserted()) {
        fatMountSimple("sd", get_io_dsisd());

        if (!isFlashcart()) {
            chdir("sd:/");
            _fsRoot = "sd:";
        }
    }
}

bool cFSManager::isFlashcart() const {
    return _isFlashcart;
}

bool cFSManager::isSDInserted() const {
    return _isSDInserted;
}

bool cFSManager::fileExists(const std::string& filePath) const {
    struct stat buffer;
    if (stat(filePath.c_str(), &buffer) != 0) {
        return false;
    }
    
    return S_ISREG(buffer.st_mode);
}

std::vector<std::string> cFSManager::getUiNames() const {
    std::vector<std::string> values;

    struct dirent* entry;
    DIR* dir = opendir((SFN_UI_DIRECTORY).c_str());
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            std::string lfn(entry->d_name);
            if (lfn != ".." && lfn != ".") values.push_back(lfn);
        }
        closedir(dir);
        dir = NULL;
    } else {
        values.push_back(gs().uiName);
    }

    std::sort(values.begin(), values.end());

    return values;
}

std::vector<std::string> cFSManager::getLangNames() const {
    std::vector<std::string> values;

    struct dirent* entry;
    DIR* dir = opendir((SFN_LANGUAGE_DIRECTORY).c_str());
    if (NULL != dir) {
        while ((entry = readdir(dir)) != NULL) {
            std::string lfn(entry->d_name);
            if (lfn != ".." && lfn != ".") values.push_back(lfn);
        }
        closedir(dir);
        dir = NULL;
    } else {
        values.push_back(gs().langDirectory);
    }
    
    std::sort(values.begin(), values.end());

    return values;
}

std::string cFSManager::resolveSystemPath(const char* path) const {
    return _fsRoot + path;
}

std::string cFSManager::getFSRoot() const {
    return _fsRoot + "/";
}

bool cFSManager::checkSDInserted() const {
    fifoSendValue32(FIFO_USER_01, MENU_MSG_IS_SD_INSERTED);

    fifoWaitValue32(FIFO_USER_06);

    int result = fifoGetValue32(FIFO_USER_06);

    return result != 0;
}
