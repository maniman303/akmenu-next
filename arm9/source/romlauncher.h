/*
    romlauncher.h
    Copyright (C) 2010 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <functional>
#include "dsrom.h"

enum TLaunchResult {
    ELaunchRomOk,
    ELaunchSDOnly,
    ELaunchRestoreFail,
    ELaunchSlowSD,
    ELaunchNoFreeSpace
};

TLaunchResult launchRom(std::string aFullPath, const DSRomInfo& aRomInfo, bool aMenu, std::string savesPath);
TLaunchResult launchRom(std::string aFullPath, const DSRomInfo& aRomInfo, bool aMenu, std::string savesPath, std::function<void()> onCompleted);
void autoLaunchRom(const std::string& aFullPath, std::function<void()> onCompleted);
