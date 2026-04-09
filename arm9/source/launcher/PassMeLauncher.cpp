/*
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <nds/ndstypes.h>
#include <string>
#include <vector>

#include "PassMeLauncher.h"
#include "nds_loader_arm9.h"

std::unique_ptr<TaskWorker> PassMeLauncher::task() const {
    return std::make_unique<PassMeLauncher>(*this);
}

s16 PassMeLauncher::process(s16 iter) {
    static const char passMeLoaderPath[] = "fat:/_nds/akmenunext/PassMeLoader.nds";

    if (access(passMeLoaderPath, F_OK) != 0) {
        showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), passMeLoaderPath));
        return -1;
    }

    std::vector<const char*> argv;
    argv.push_back(passMeLoaderPath);
    runNdsFile(argv[0], argv.size(), &argv[0]);

    return -1;
}
