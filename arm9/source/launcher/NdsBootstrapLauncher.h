/*
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds/ndstypes.h>

#include "../dsrom.h"
#include "Launcher.h"

class NdsBootstrapLauncher : public Launcher {
  public:
    std::unique_ptr<TaskWorker> task() const override;
    bool process() override;

  private:
    bool prepareCheats(const std::string& mRomPath);
    bool prepareIni(const std::string& mRomPath, const std::string& mSavePath, bool hb);
    bool is3DS(void);
    bool _messageBlock;
    bool _useNightly;
    std::string _mRomPath;
    std::string _mSavePath;
    u32 _mFlags;
    DSRomInfo _romInfo;
    std::vector<const char*> _argv;
};
