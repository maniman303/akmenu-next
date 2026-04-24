/*
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <nds/ndstypes.h>
#include <string>
#include <vector>

#include "Slot1Launcher.h"
#include "nds_loader_arm9.h"

TaskWorker* Slot1Launcher::task() {
    return this;
}

s16 Slot1Launcher::process(s16 iter) {
    static const std::string slot1LoaderPath = fsManager().resolveSystemPath("/_nds/akmenunext/slot1launch.nds");

    if (access(slot1LoaderPath.c_str(), F_OK) != 0) {
        showModalOk(LOADER_NOT_FOUND_TITLE, formatString(LOADER_NOT_FOUND_MESSAGE.c_str(), slot1LoaderPath.c_str()));
        return -1;
    }

    std::vector<const char*> argv;
    argv.push_back(slot1LoaderPath.c_str());
    runNdsFile(argv[0], argv.size(), &argv[0]);

    return -1;
}
