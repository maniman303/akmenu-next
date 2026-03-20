/*
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds/ndstypes.h>
#include <string>
#include "../language.h"
#include "../ui/msgbox.h"

static inline void printLoaderNotFound(std::string loaderPath) {
    akui::cMessageBox::showModal(NULL, LANG("loader", "not found"), loaderPath, MB_OK);
}

static inline void printError(std::string errorMsg) {
    akui::cMessageBox::showModal(NULL, LANG("loader", "error"), errorMsg, MB_OK);
}

struct MessageEntry {
    std::string title = "";
    std::string content = "";

    bool empty() const {
        return title.empty() || content.empty();
    }
};

class ILauncher {
  public:
    virtual ~ILauncher() {}
    virtual bool launchRom(std::string romPath, std::string savePath, u32 flags, u32 cheatOffset, u32 cheatSize, bool hb) = 0;
    virtual MessageEntry prepareLaunchMessage() { return MessageEntry{}; }
};
