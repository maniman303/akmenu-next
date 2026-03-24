/*
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <nds/ndstypes.h>
#include <string>
#include <vector>

#include "flags.h"
#include "HomebrewLauncher.h"
#include "nds_loader_arm9.h"

std::unique_ptr<TaskWorker> HomebrewLauncher::task() const {
    return std::make_unique<HomebrewLauncher>(*this);
}

bool HomebrewLauncher::process() {
    std::vector<const char*> argv;
    argv.push_back(_romPath.c_str());
    runNdsFile(argv[0], argv.size(), &argv[0]);

    return true;
}
