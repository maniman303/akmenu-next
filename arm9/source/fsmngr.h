#pragma once
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "singleton.h"

class cFSManager {
public:
    cFSManager();
    void init(int argc, char* argv[]);
    bool isFlashcart() const;
    bool isSDInserted() const;
    bool isRebooted() const;
    bool fileExists(const std::string& filePath) const;
    std::string getFilename(const std::string& path) const;
    std::vector<std::string> getUiNames() const;
    std::vector<std::string> getLangNames() const;
    std::string resolveSystemPath(const char* path) const;
    std::string getFSRoot() const;
    const std::string& getIconPath(const std::string& iconName);

private:
    bool checkSDInserted() const;
    bool _isSDInserted;
    bool _isFlashcart;
    bool _isRebooted;
    std::string _fsRoot;
    std::string _lastTheme;
    std::unordered_map<std::string, std::string> _iconMap;
};


typedef t_singleton<cFSManager> FSManager_s;
inline cFSManager& fsManager() {
    return FSManager_s::instance();
}
