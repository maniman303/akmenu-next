/*
    formdesc.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <string>
#include "bmp15.h"
#include "gdi.h"
#include "point.h"
#include "rectangle.h"

class cFormDesc {
  public:
    cFormDesc();

    ~cFormDesc();

  public:
    void draw(const cRect& area, GRAPHICS_ENGINE engine) const;

    void loadData(const std::string& topleftBmpFile, const std::string& toprightBmpFile,
                  const std::string& middleBmpFile);

    cSize titleSize();

    void setTitleText(const std::string& text);

    void setTitleText(const std::string& text, bool centerTitle);

  protected:
    cBMP15 _topleft;
    cBMP15 _middle;
    cBMP15 _topright;

    COLOR _bodyColor;
    COLOR _bodyColor2;
    COLOR _frameColor;

    std::string _titleText;
    bool _centerTitle;
};
