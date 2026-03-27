#pragma once

#include <unordered_map>
#include "inifile.h"
#include "singleton.h"

class cCachedIniFile {
  public:
    cCachedIniFile();

    ~cCachedIniFile();

    CIniFile get(const std::string& filename);

    void clear();
  private:
    std::unordered_map<std::string, CIniFile> _cache;
};

typedef t_singleton<cCachedIniFile> CachedIniFile_s;
inline cCachedIniFile& iniFiles() {
    return CachedIniFile_s::instance();
}