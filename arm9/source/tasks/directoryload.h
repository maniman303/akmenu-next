#pragma once

#include <nds.h>
#include <vector>
#include <deque>
#include <queue>
#include <unordered_set>
#include <string>
#include <dirent.h>
#include "taskworker.h"

class DirectoryLoadTask : public TaskWorker {
  public:
    struct DirEntry {
      enum DIR_STATE { NORMAL = 0, FAVORITE = 1, LAST = 2 };

      std::string showName;
      std::string internalName;
      std::string fullPath;
      DIR_STATE state;

      DirEntry(const std::string& s, const std::string& i, const std::string& f, const DIR_STATE& d) {
        showName = s;
        internalName = i;
        fullPath = f;
        state = d;
      }

      DirEntry(const std::string& s, const std::string& i, const std::string& f) : DirEntry(s, i, f, DIR_STATE::NORMAL) {}

      DirEntry() : DirEntry("", "", "", DIR_STATE::NORMAL) {}
    };

    DirectoryLoadTask(std::string dirName, std::function<void(std::deque<DirEntry>&)> onLoadCompleted);
    ~DirectoryLoadTask();

    s16 process(s16 iter) override;
    void setOnCompleted(std::function<void()> onCompleted) override;
    void schedule();

  private:
    enum STAGE { DEF = 0, SCAN = 1, FAV = 2, PATH = 3, LAST = 4 };

    std::unordered_set<std::string>& getFavorites();
    bool setupLastPlayed();
    bool setupFavorites();
    bool setupDefaultDir();
    bool setupGameScan();
    bool setupPath();

    std::queue<s16> _plan;
    std::unordered_set<s16> _completed;
    std::string _dirName;
    std::function<void(std::deque<DirEntry>&)> _onLoadCompleted;
    std::function<void()> _extraOnCompleted;
    std::deque<DirEntry> _data;
    std::unordered_set<std::string> _favorites;
    std::unordered_set<std::string>::iterator _favoritesIter;
    std::queue<std::string> _pathsToScan;
    DIR* _scanDir;
    DIR* _pathDir;
};