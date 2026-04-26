#pragma once

#include <nds.h>
#include <string>
#include <functional>
#include "../dsrom.h"
#include "ui/statictext.h"
#include "ui/window.h"

class cRomBootWnd : public akui::cWindow {
  public:
    cRomBootWnd(std::string romPath, std::function<void()> onExit);

    akui::cWindow& loadAppearance(const std::string& aFileName) override;
    bool processKeyMessage(cKeyMessage message) override;
    bool processTouchMessage(cTouchMessage message) override;
    void update() override;

  protected:
    void onGainedFocus() override;
    void draw() override;
    void drawBackdrop() override;

  private:
    void moveToMain();
    std::string _romPath;
    DSRomInfo _romInfo;
    std::function<void()> _onExit;
    akui::cStaticText _nameText;
    akui::cStaticText _continueText;
};