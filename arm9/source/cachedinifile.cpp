#include "cachedinifile.h"

cCachedIniFile::cCachedIniFile() {
    _cache = std::unordered_map<std::string, CIniFile>();
}

cCachedIniFile::~cCachedIniFile() {
    clear();
}

CIniFile cCachedIniFile::get(const std::string& filename) {
    auto it = _cache.find(filename);
    if (it != _cache.end()) {
        return it->second;
    }

    CIniFile ini(filename);
    _cache[std::move(filename)] = std::move(ini);

    return ini;
}

void cCachedIniFile::clear() {
    _cache.clear();
    _cache.rehash(0);
}