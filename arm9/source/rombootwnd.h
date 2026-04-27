#pragma once

#include <nds.h>
#include <string>
#include <functional>
#include "dsrom.h"
#include "ui/animation.h"
#include "ui/statictext.h"
#include "ui/window.h"

class cRomBootWnd : public cWindow {
  public:
    cRomBootWnd(std::string romPath, std::function<void()> onExit);
    ~cRomBootWnd() override;

    cWindow& loadAppearance(const std::string& aFileName) override;
    bool processKeyMessage(cKeyMessage message) override;
    bool processTouchMessage(cTouchMessage message) override;
    void update() override;

  protected:
    void onFocused() override;
    void draw() override;
    void drawBackdrop() override;

  private:
    void moveToMain();
    void startRom();
    std::string _romPath;
    DSRomInfo _romInfo;
    std::function<void()> _onExit;
    u16 _timer;
    Animation _pressAnimation;
    cStaticText _launchText;
    cStaticText _nameText;
    cStaticText _pressText;
};