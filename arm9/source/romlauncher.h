/*
    romlauncher.h
    Copyright (C) 2010 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "dsrom.h"
#include "functional"

enum TLaunchResult {
    ELaunchRomOk,
    ELaunchSDOnly,
    ELaunchRestoreFail,
    ELaunchSlowSD,
    ELaunchNoFreeSpace
};

TLaunchResult launchRom(const std::string& aFullPath, DSRomInfo& aRomInfo, bool aMenu, const std::string& savesPath);
TLaunchResult launchRom(const std::string& aFullPath, DSRomInfo& aRomInfo, bool aMenu, const std::string& savesPath, std::function<void()> onCompleted);
void autoLaunchRom(const std::string& aFullPath, std::function<void()> onCompleted);
