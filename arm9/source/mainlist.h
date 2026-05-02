/*
    mainlist.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds.h>
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
        IS_FAVORITE_COLUMN = 6,
        ROMINFO_COLUMN = 7
    };

  public:
    cMainList(cWindow* parent, const std::string& text);

    ~cMainList();

  public:
    int init();
    bool enterDir(const std::string& dirName);
    void backParentDir();
    bool processKeyMessage(cKeyMessage message) override;
    cRect focusRectangle() const override;
    std::string getCurrentDir();
    bool getRomInfo(u32 rowIndex, DSRomInfo& info) const;
    void setRomInfo(u32 rowIndex, const DSRomInfo& info);
    void selectRom(const std::string& romPath);
    void setViewMode(VIEW_MODE mode);
    std::string getRowFullPath(u32 id);
    std::string getRowShowName(u32 id);
    std::string getRowFileName(u32 id);
    u32 getRowIdByPath(std::string path);
    VIEW_MODE getViewMode() { return _viewMode; }

    Signal0 directoryChanged;

  public:
    bool IsFavorites(void);
    void SwitchShowAllFiles(void);
    const std::vector<std::string>* Saves(void);

  protected:
    void draw() override;
    void drawBackdrop() override;
    void drawIcons();
    void drawItemBackgrounds();
    void updateInternalNames(void);
    bool insertEntryRow(const std::vector<std::string>& texts, const DSRomInfo& romInfo);
    void processDirIcons();
    bool setupDefaultDir(bool skipCards, bool skipFavorites);
    bool setupGameDir();
    void onDirectoryChanged();
    std::vector<std::string> getLastPlayedRow();
    std::vector<std::vector<std::string>> getFavoriteRows(bool exclusive);
    std::vector<std::vector<std::string>> getGameRows(int rowsToLoad);

  protected:
    void onSelectChanged(u32 index);
    void onScrolled(u32 index);
    std::string processItemText(std::string text, int column);

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
    bool _showAllFiles;
    std::vector<std::string> _saves;

  protected:
    u32 _topCount;
    u32 _topuSD;
    u32 _topuDSiSD;
    u32 _topFavorites;
    u32 _topSlot1;
    u32 _topSlot2;

  public:
    u32 Slot1();
    u32 Slot2();
    u32 SDCard();
};
