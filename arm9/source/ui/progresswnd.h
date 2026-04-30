/*
    progresswnd.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "form.h"
#include "progressbar.h"
#include "statictext.h"
#include "singleton.h"

namespace akui {

  class cProgressWnd : public cForm {
    public:
      cProgressWnd();  // s32 x, s32 y, u32 w, u32 h, cWindow * parent, const std::string & text );

      ~cProgressWnd();

    public:
      void init();
      void draw();
      bool processKeyMessage(cKeyMessage message) override;
      void setPercent(u8 percent);
      void setTipText(const std::string& tipText);

    protected:
      void onShow();
      void onHide();

      cProgressBar _bar;
      cStaticText _tip;
      cBMP15 _background;
  };

}  // namespace akui

typedef t_singleton<akui::cProgressWnd> progressWnd_s;
inline akui::cProgressWnd& progressWnd() {
    return progressWnd_s::instance();
}
