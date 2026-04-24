/*
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <string>

#include "Launcher.h"

class HomebrewLauncher : public Launcher {
  public:
    TaskWorker* task() override;
    s16 process(s16 iter) override;
};
