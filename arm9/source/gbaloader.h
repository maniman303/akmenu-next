/*
    gbaloader.h
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds.h>
#include <string>

class CGbaLoader {
  public:
    static void StartGBA(void);
    static u8 GetGbaHeader();

  private:
    static void LoadBorder(void);
    static void BootGBA(void);
};
