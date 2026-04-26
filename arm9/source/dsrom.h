/*
    dsrom.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds.h>
#include <string>
#include <memory>
#include "gdi.h"
#include "savemngr.h"

class DSRomInfo {
  private:
    enum TBool { ETrue, EFalse, EMayBe };

  private:
    tNDSBanner _banner;
    SAVE_INFO_EX _saveInfo;
    TBool _isDSRom;
    TBool _isDSiWare;
    TBool _isHomebrew;
    TBool _isModernHomebrew;
    TBool _isGbaRom;
    std::string _fileName;
    s32 _extIcon;
    u8 _romVersion;
    std::shared_ptr<u32[]> _buffer;
    bool _lastSize;

  private:
    void load(void);
    bool loadGbaRomInfo(const std::string& filename);
    bool loadDSRomInfo(const std::string& filename, bool loadBanner);
    void drawDSRomIconMem(u16* mem, bool small);

  public:
    DSRomInfo();
    ~DSRomInfo();

  public:
    void drawDSRomIcon(u8 x, u8 y, bool small, GRAPHICS_ENGINE engine);
    std::string getDsLocTitle();
    tNDSBanner& banner(void);
    SAVE_INFO_EX& saveInfo(void);
    u8 version(void);
    void setExtIcon(const std::string& aValue);
    inline bool isExtIcon(void) { return _extIcon >= 0; };
    bool isDSRom(void);
    bool isDSiWare(void);
    bool isHomebrew(void);
    bool isModernHomebrew(void);
    bool isGbaRom(void);
    DSRomInfo& operator=(const DSRomInfo& src);
    void mayBeDSRom(const std::string& filename) {
        _isDSRom = EMayBe;
        _isHomebrew = EMayBe;
        _isModernHomebrew = EMayBe;
        _isDSiWare = EMayBe;
        _fileName = filename;
        load();
    };
    void mayBeGbaRom(const std::string& filename) {
        _isGbaRom = EMayBe;
        _fileName = filename;
        load();
    };
    bool setBannerFromFile(const std::string& anExtIcon, const std::string& path, const u8* aBanner);
    bool lastSize();
};
