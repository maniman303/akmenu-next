/*
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds/ndstypes.h>

#include "Launcher.h"
#include "PicoLoader/picoLoader7.h"

class PicoLoaderLauncher : public Launcher {
  public:
    TaskWorker* task() override;
    s16 process(s16 iter) override;

  private:
    void copyToVram(const char* loaderPath, void* destination);
    bool prepareCheats(void);
    bool setParameters(void);
    pload_cheats_t* mCheats;
    FILE* _loader7;
    FILE* _loader9;
    pload_params_t _sLoadParams;
    s64 _picoLoader7Size;
    s64 _picoLoader9Size;
    u8* _picoLoader7;
    u8* _picoLoader9;
};