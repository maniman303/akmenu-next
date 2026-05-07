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

    std::string out;
    std::string line;

    out.reserve(text.size() + text.size() / 16);

    const char* p = text.c_str();
    size_t len = text.size();

    u32 width = 0;

    size_t colonBreak = std::string::npos;
    size_t spaceBreak = std::string::npos;

    for (size_t i = 0; i < len;) {
        u32 ww, add;
        Info(p + i, &ww, &add);

        char c = p[i];

        if (c == '\n') {
            out += line;
            out += '\n';

            line.clear();

            width = 0;
            colonBreak = std::string::npos;
            spaceBreak = std::string::npos;

            i++;
            continue;
        }

        if (ww == 0 || c == '\r') {
            i++;
            continue;
        }

        line.append(p + i, add);
        width += ww;

        if (c == ':') {
            colonBreak = line.size() - add;
        } else if (c == ' ') {
            spaceBreak = line.size() - add;
        }

        if (width > maxLineWidth) {
            size_t breakPos = std::string::npos;

            // ':' has priority
            if (colonBreak != std::string::npos) {
                breakPos = colonBreak + 1;
            } else if (spaceBreak != std::string::npos) {
                breakPos = spaceBreak;
            }

            if (breakPos != std::string::npos) {
                out.append(line.data(), breakPos);
                out += '\n';

                size_t next = breakPos + 1;

                while (next < line.size()) {
                    char s = line[next];

                    if (s != ' ' && s != '\t')
                        break;

                    next++;
                }

                line.erase(0, next);
            } else {
                out += line;
                out += '\n';
                line.clear();
            }

            // Recalculate width
            width = 0;

            for (size_t k = 0; k < line.size();) {
                u32 cw, ca;
                Info(line.c_str() + k, &cw, &ca);

                width += cw;
                k += ca;
            }

            colonBreak = std::string::npos;
            spaceBreak = std::string::npos;
        }

        i += add;
    }

    out += line;

    return out;
}

u32 cFont::FontRAM(void) {
    return 0;
}
