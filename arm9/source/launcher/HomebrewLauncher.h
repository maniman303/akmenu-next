/*
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <string>

#include "Launcher.h"

class HomebrewLauncher : public Launcher {
  public:
    std::unique_ptr<TaskWorker> task() const override;
    s16 process(s16 iter) override;
};
