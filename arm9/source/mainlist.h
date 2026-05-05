/*
    mainlist.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds.h>
#include <deque>
#include "dsrom.h"
#include "keymessage.h"
#include "listview.h"
#include "sigslot.h"
#include "touchmessage.h"
#include "image.h"

class cMainList : public akui::cListView {
  public:
    enum VIEW_MODE { VM_LIST = 0, VM_ICON, VM_INTERNAL, VM_LIST_ICON };

    enum  // COLUMN_LIST
    {
        ICON_COLUMN = 0,
        SHOWNAME_COLUMN = 1,
        INTERNALNAME_COLUMN = 2,
        REALNAME_COLUMN = 3,
        SAVETYPE_COLUMN = 4,
        FILESIZE_COLUMN = 5,
        IS_FAVORITE_COLUMN = 6
    };

  public:
    cMainList(cWindow* parent, const std::string& text);

    ~cMainList();

  public:
    int init();
    bool enterDir(const std::string& dirName);
    bool backParentDir();
    void update() override;
    bool processKeyMessage(cKeyMessage message) override;
    cRect focusRectangle() const override;
    std::string getCurrentDir();
    bool getRomInfo(u32 rowIndex, DSRomInfo& info);
    void scheduleRomSelection(const std::string& romPath);
    void setViewMode(VIEW_MODE mode);
    std::string getRowFullPath(u32 id) const;
    std::string getRowShowName(u32 id) const;
    std::string getRowFileName(u32 id) const;
    u32 getRowIdByPath(std::string path);
    VIEW_MODE getViewMode() { return _viewMode; }

    Signal0 directoryChanged;
    Signal0 directoryReturned;

  protected:
    void draw() override;
    void drawBackdrop() override;
    void drawIcons();
    void drawItemBackgrounds();
    bool insertEntryRow(const std::vector<std::string>& texts);
    void processDirIcon(DSRomInfo& romInfo, const std::string fileName);
    void processDirIcons();
    void validateDirIcons();
    void onScrolled(u32 index) override;
    void onDirectoryChanged(std::deque<std::vector<std::string>>& rows, std::string dirName);
    void selectRom(const std::string& romPath);
    void selectRom(const std::string& romPath, bool silent);
    std::vector<std::vector<std::string>> setupDefaultDir(bool skipCards, bool skipFavorites);
    std::vector<std::vector<std::string>> setupGameDir();
    std::vector<std::vector<std::string>> prepareDir(const std::string& dirName);
    std::vector<std::string> getLastPlayedRow();
    std::vector<std::vector<std::string>> getFavoriteRows(bool exclusive);
    std::vector<std::vector<std::string>> getGameRows(int rowsToLoad);

  protected:
    std::string processItemText(std::string text, int column);
    u32 slotSDCard();
    u32 slotFavorites();

  protected:
    int _iconPrefix;
    int _iconSufix;
    int _textPrefix;
    int _textSufix;
    int _tallRowHeight;
    bool _centerInternalColumn;
    akui::cImage* _itemBg;
    VIEW_MODE _viewMode;
    std::string _currentDir;
    std::vector<std::string> _extnameFilter;
    std::vector<DSRomInfo> _romInfoList;
    bool _busy;
    std::string _scheduledRom;
};
