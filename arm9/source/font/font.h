/*
    font.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <string>
#include <nds.h>
#include "singleton.h"

class cFont {
  public:
    cFont();
    virtual ~cFont();

  public:
    std::string BreakLine(const std::string& text, u32 maxLineWidth);

  public:
    virtual void SetHeight(u8 height) = 0;
    virtual u8 GetHeight() const = 0;
    virtual void SetDescend(u8 descend) = 0;
    virtual u8 GetDescend() const = 0;
    virtual std::string GetFilename() const = 0;
    virtual void Info(const char* aString, u32* aWidth, u32* aSymbolCount) const = 0;
    virtual u32 TextWidth(const std::string& aString) const = 0;
    virtual void Draw(u16* mem, s16 x, s16 y, const u8* aText, u16 color) const = 0;
    virtual bool Load(const char* aFileName) = 0;
    virtual u32 FontRAM(void);
};
