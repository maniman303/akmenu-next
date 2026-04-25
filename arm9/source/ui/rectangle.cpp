/*
    rectangle.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "rectangle.h"

cRect::cRect(const cPoint& p1, const cPoint& p2) : cRect(p1, p2, true) { }

cRect::cRect(const cPoint& p1, const cPoint& p2, bool usePoints) {
    if (!usePoints) {
        _position = p1;
        _size = p2;
        return;
    }

    if (p1.x < p2.x) {
        _position.x = (p1.x);
        _size.x = (p2.x - _position.x);
    } else {
        _position.x = (p2.x);
        _size.x = (p1.x - _position.x);
    }

    if (p1.y < p2.y) {
        _position.y = (p1.y);
        _size.y = (p2.y - _position.y);
    } else {
        _position.y = (p2.y);
        _size.x = (p1.y - _position.y);
    }
}

cRect::cRect(int _x1, int _y1, int _x2, int _y2) {
    (*this) = cRect(cPoint(_x1, _y1), cPoint(_x2, _y2));
}

cRect& cRect::translateBy(const cPoint& p) {
    _position += p;
    return *this;
}

cPoint cRect::center() const {
    return _position + halfSize();
}

cPoint cRect::bottomLeft() const {
    return cPoint(minX(), maxY());
}

cPoint cRect::bottomRight() const {
    return _position + _size;
}

cPoint cRect::topLeft() const {
    return _position;
}

cPoint cRect::topRight() const {
    return cPoint(maxX(), minY());
}

bool cRect::isAboveAndBelow(const cPoint& p) const {
    return ((p.y >= _position.y) && (p.y <= _position.y + _size.y));
}

bool cRect::isLeftAndRightOf(const cPoint& p) const {
    return ((p.x >= _position.x) && (p.x <= _position.x + _size.x));
}

bool cRect::surrounds(const cPoint& p) const {
    return isAboveAndBelow(p) && isLeftAndRightOf(p);
}

cRect& cRect::expandBy(int amount) {
    _position -= cPoint(amount, amount);
    _size += cPoint(amount * 2, amount * 2);
    return *this;
}

cRect& cRect::expandWidthBy(int amount) {
    _position -= cPoint(amount, 0);
    _size += cPoint(amount * 2, 0);

    return *this;
}

cRect& cRect::expandHeightBy(int amount) {
    _position -= cPoint(0, amount);
    _size += cPoint(0, amount * 2);

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
