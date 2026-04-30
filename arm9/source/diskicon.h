/*
    diskicon.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds.h>
#include "bmp15.h"
#include "singleton.h"
#include "window.h"

class cDiskIcon : public cWindow {
  public:
    cDiskIcon();

    ~cDiskIcon() {}

  public:
    void draw() override;
    void turnOn();
    void turnOff();
    void loadAppearance(const std::string& aFileName);
    void blink(void);

  protected:
    bool _show;
    bool _blink;
    float _lightTime;
    cBMP15 _icon;
};

typedef t_singleton<cDiskIcon> diskIcon_s;
inline cDiskIcon& diskIcon() {
    return diskIcon_s::instance();
}
