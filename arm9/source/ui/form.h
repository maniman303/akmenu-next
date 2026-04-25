/*
    form.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <list>
#include <functional>
#include "rectangle.h"
#include "window.h"

namespace akui {

  class cForm : public cWindow {
    public:
      cForm(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text);

      ~cForm() override;

    public:
      cForm& addChildWindow(cWindow* aWindow);
      cForm& removeChildWindow(cWindow* aWindow);
      bool canRenderBackdrop() override;
      bool shouldRenderBackdrop() override;
      void onRenderBackdrop() override;
      void draw() override;
      void drawBackdrop() override;
      bool processKeyMessage(cKeyMessage message) override;
      bool processTouchMessage(cTouchMessage message) override;
      cRect focusRectangle() const override;
      cWindow* windowBelow(const cPoint& p);
      u32 modalRet();
      void centerScreen();
      bool hasFocus() const override;
      cWindow& disableFocusable(void) override;

      std::function<void()> onAccepted;
      std::function<void()> onRejected;

    protected:
      virtual void onOK();
      virtual void onCancel();
      std::list<cWindow*> _childWindows;

      u32 _modalRet;
  };

}  // namespace akui
