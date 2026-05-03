/*
    popmenu.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds.h>
#include <vector>
#include "form.h"
#include "point.h"
#include "sigslot.h"
#include "window.h"

namespace akui {

  class cPopMenu : public cWindow {
    public:
      cPopMenu(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text);

      ~cPopMenu();

    public:
      void draw() override;
      void loadAppearance(const std::string& aFileName);
      bool processKeyMessage(cKeyMessage message) override;
      bool processTouchMessage(cTouchMessage message) override;
      void popup();
      void addItem(size_t index, const std::string& itemText);
      void removeItem(size_t index);
      size_t itemCount();
      void clearItem();

      Signal1<s16> itemClicked;
      Signal1<s16> itemSelected;
      Signal1<bool> menuExit;

    protected:
      void onShow() override;
      void onExit() override;
      void close(bool selected);
      void selectItem(s16 item, bool silent);
      s32 itemBelowPoint(const cPoint& pt);
      void drawItems();
      s16 barWidth(void);

      cPoint _itemTopLeftPoint;
      std::vector<std::string> _items;
      s16 _selectedItemIndex;
      s16 _itemHeight;
      s16 _itemWidth;
      s16 _barLeft;
      COLOR _textColor;
      COLOR _textHighLightColor;
      COLOR _barColor;
      cBMP15 _background;
      bool _skipTouch;
  };

}  // namespace akui
