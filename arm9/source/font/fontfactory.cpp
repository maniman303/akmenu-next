/*
    fontfactory.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "fontfactory.h"
#include "font_pcf.h"
#include "language.h"
#include "stringtool.h"
#include "systemfilenames.h"

cFontFactory::cFontFactory() : _font(NULL) {}

cFontFactory::~cFontFactory() {
    if (_font != NULL) delete _font;
    if (_fontSecondary != NULL) delete _fontSecondary;
}

void cFontFactory::makeFont(void) {
    std::string filename = lang().GetString("font", "main", SFN_DEFAULT_FONT);
    u8 height = lang().GetInt("font", "main height", gs().fontHeight);
    u8 descend = lang().GetInt("font", "main descend", 0);
    std::string filepath(SFN_FONTS_DIRECTORY + filename);
    _font = new cFontPcf();
    _font->Load(filepath.c_str());
    _font->SetHeight(height);
    _font->SetDescend(descend);

    std::string filenameSecondary = lang().GetString("font", "secondary", "");
    std::string filepathSecondary(SFN_FONTS_DIRECTORY + lang().GetString("font", "secondary", ""));
    _fontSecondary = new cFontPcf();
    if (!_fontSecondary->Load(filepathSecondary.c_str())) {
        delete _fontSecondary;
        _fontSecondary = NULL;
        return;
    }

    u8 secondaryHeight = lang().GetInt("font", "secondary height", gs().fontHeight);
    u8 secondaryDescend = lang().GetInt("font", "secondary descend", 0);
    _fontSecondary->SetHeight(secondaryHeight);
    _fontSecondary->SetDescend(secondaryDescend);
}
