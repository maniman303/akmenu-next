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

    cGbaLoader();

    void init();
    bool validate();
    bool startRom(const std::string& fileName);

  private:
    static void LoadBorder(void);
    static void BootGBA(void);

    bool tryCopyBorder();
    bool setupSettings();

    bool _isRunnerPresent;
    bool _isBiosPresent;
};

typedef t_singleton<cGbaLoader> GbaLoader_s;
inline cGbaLoader& gbaLoader() {
    return GbaLoader_s::instance();
}