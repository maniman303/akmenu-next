/*
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds/ndstypes.h>

#include "Launcher.h"
#include "DSpico/picoLoader7.h"

class DSpicoLauncher : public Launcher {
  public:
    std::unique_ptr<TaskWorker> task() const override;
    bool process() override;

  private:
    FILE* _loader7;
    FILE* _loader9;
    pload_params_t _sLoadParams;
    s64 _picoLoader7Size;
    s64 _picoLoader9Size;
    u8* _picoLoader7;
    u8* _picoLoader9;
};