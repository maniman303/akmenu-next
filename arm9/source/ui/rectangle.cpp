/*
    rectangle.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "rectangle.h"

namespace akui {

    cRect::cRect(const cPoint& p1, const cPoint& p2) : cRect(p1, p2, true) { }

    cRect::cRect(const cPoint& p1, const cPoint& p2, bool usePoints) {
        if (!usePoints) {
            position_ = p1;
            size_ = p2;
            return;
        }

        if (p1.x < p2.x) {
            position_.x = (p1.x);
            size_.x = (p2.x - position_.x);
        } else {
            position_.x = (p2.x);
            size_.x = (p1.x - position_.x);
        }

        if (p1.y < p2.y) {
            position_.y = (p1.y);
            size_.y = (p2.y - position_.y);
        } else {
            position_.y = (p2.y);
            size_.x = (p1.y - position_.y);
        }
    }

    cRect::cRect(int _x1, int _y1, int _x2, int _y2) {
        (*this) = cRect(cPoint(_x1, _y1), cPoint(_x2, _y2));
    }

    cRect& cRect::translateBy(const cPoint& p) {
        position_ += p;
        return *this;
    }

    cPoint cRect::bottomLeft() const {
        return cPoint(minX(), maxY());
    }

    cPoint cRect::bottomRight() const {
        return position_ + size_;
    }

    cPoint cRect::topLeft() const {
        return position_;
    }

    cPoint cRect::topRight() const {
        return cPoint(maxX(), minY());
    }

    cPoint cRect::centerPoint() const {
        return position_ + halfSize();
    }

    bool cRect::isAboveAndBelow(const cPoint& p) const {
        return ((p.y >= position_.y) && (p.y <= position_.y + size_.y));
    }

    bool cRect::isLeftAndRightOf(const cPoint& p) const {
        return ((p.x >= position_.x) && (p.x <= position_.x + size_.x));
    }

    bool cRect::surrounds(const cPoint& p) const {
        return isAboveAndBelow(p) && isLeftAndRightOf(p);
    }

    cRect& cRect::expandBy(int amount) {
        position_ -= cPoint(amount, amount);
        size_ += cPoint(amount * 2, amount * 2);
        return *this;
    }

    cRect& cRect::expandWidthBy(int amount) {
        position_ -= cPoint(amount, 0);
        size_ += cPoint(amount * 2, 0);

        return *this;
    }

    cRect& cRect::expandHeightBy(int amount) {
        position_ -= cPoint(0, amount);
        size_ += cPoint(0, amount * 2);

        return *this;
    }

    int cRect::height() const {
        return size().y;
    }

    int cRect::width() const {
        return size().x;
    }

    bool cRect::operator==(const cRect& rect) const {
        return ((position() == rect.position()) && (size() == rect.size()));
    }

    bool cRect::operator!=(const cRect& rect) const {
        return !(*this == rect);
    }

}  // namespace akui
