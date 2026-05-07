/*
    gbaloader.h
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds.h>
#include <string>
#include "singleton.h"

class cGbaLoader {
  public:
    static void StartGBA(void);
    static u8 GetGbaHeader();

  private:
    static void LoadBorder(void);
    static void BootGBA(void);
};

typedef t_singleton<cGbaLoader> GbaLoader_s;
inline cGbaLoader& gbaLoader() {
    return GbaLoader_s::instance();
}