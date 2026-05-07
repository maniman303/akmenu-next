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
    if (text.empty() || maxLineWidth == 0) return text;

    std::string ret;
    ret.reserve(text.size() + text.size() / 20);

    const char* p = text.c_str();
    const size_t len = text.size();

    size_t lineStart = 0;
    size_t i = 0;

    u32 lineWidth = 0;

    struct BreakPoint {
        size_t posInText;
        size_t posInRet;
        u32 widthAfter;
        int priority;
        bool valid;
    };
    BreakPoint bp = { 0, 0, 0, 0, false };

    auto flushUpTo = [&](size_t end) {
        ret.append(p + lineStart, p + end);
        lineStart = end;
    };

    while (i < len) {
        u32 ww, add;
        Info(p + i, &ww, &add);

        if (*(p + i) == '\n') {
            flushUpTo(i + 1);
            lineWidth = 0;
            bp.valid = false;
            i++;
            continue;
        }

        if (ww == 0 || *(p + i) == '\r') {
            i++;
            continue;
        }

        lineWidth += ww;

        if (*(p + i) == ' ' && (!bp.valid || bp.priority <= 1)) {
            bp = { i, ret.size() + (i - lineStart), lineWidth, 1, true };
        } else if (*(p + i) == ':' && (!bp.valid || bp.priority <= 2)) {
            bp = { i, ret.size() + (i - lineStart), lineWidth, 2, true };
        }

        if (lineWidth > maxLineWidth) {
            if (bp.valid) {
                flushUpTo(bp.posInText);

                if (bp.priority == 1) {
                    ret.push_back('\n');
                    lineStart = bp.posInText + 1;
                } else {
                    ret.append(p + lineStart, p + lineStart + add);
                    ret.push_back('\n');
                    lineStart = bp.posInText + add;
                }

                lineWidth = lineWidth - bp.widthAfter;
                bp.valid = false;
            } else {
                flushUpTo(i);
                ret.push_back('\n');
                lineWidth = ww;
                bp.valid = false;
            }
        }

        i += add;
    }

    flushUpTo(len);

    if (!ret.empty() && ret.back() == '\n')
        ret.pop_back();

    return ret;
}

u32 cFont::FontRAM(void) {
    return 0;
}
