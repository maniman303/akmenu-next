/*
    Copyright (C) 2024 lifehackerhansol

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds/ndstypes.h>
#include <string>
#include "Launcher.h"

class TopToyLauncher : public Launcher {
  public:
    std::unique_ptr<TaskWorker> task() const override;
    s16 process(s16 iter) override;

  private:
    bool prepareCheats(void);
    bool prepareTTSYS(void);
    std::string mRomPath;
    std::string mSavePath;
    u32 mFlags;
};
