/*
    font.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "font.h"
#include <nds.h>
#include <string.h>
#include "dbgtool.h"
#include "systemfilenames.h"

cFont::cFont() {}

cFont::~cFont() {}

std::string cFont::BreakLine(const std::string& text, u32 maxLineWidth) {
    if (text.empty() || maxLineWidth == 0) {
        return text;
    }

    std::string ret;

    const char* p = text.c_str();
    bool hasSpace = false;
    u32 tempWidth = 0;
    u32 lastTempWidth = 0;

    while (*p != 0) {
        u32 ww, add;
        Info(p, &ww, &add);
        tempWidth += ww;

        if (*p == '\n') {
            ret.push_back('\n');
            hasSpace = false;
            tempWidth = 0;
            lastTempWidth = 0;
            p++;
            continue;
        }

        if (ww == 0 || *p == '\r') {
            p++;
            continue;
        }

        if (*p == ' ') {
            hasSpace = true;
            lastTempWidth = tempWidth;
        }
        
        if (tempWidth > maxLineWidth) {
            if (hasSpace) {
                u32 lastSpacePos = ret.find_last_of(' ');
                ret[lastSpacePos] = '\n';
                tempWidth = tempWidth - lastTempWidth - 1;
                hasSpace = false;
            } else {
                ret.push_back('\n');
                tempWidth = 0;
            }
        }

        for (u32 ii = 0; ii < add; ii++) ret.push_back(*p++);
    }

    if (ret.back() == '\n') {
        ret = ret.substr(0, ret.length() - 1);
    }

    return ret;
}

u32 cFont::FontRAM(void) {
    return 0;
}
