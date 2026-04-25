/*
    gdi.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gdi.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <math.h>
#include "dbgtool.h"
#include "fontfactory.h"
#include "globalsettings.h"
#include "sprite.h"
#include "userinput.h"
#include "logger.h"
#include "blockds/ndstypes.h"
#include "blockds/video.h"
#include "../../share/memtool.h"

ARM_CODE LIBNDS_NOINLINE
static void ITCM_FUNC(dmaCopyWordsGdi)(uint8 channel, const void* src, void* dest, uint32 size) {
    DC_FlushRange(src, size);
    dmaCopyWords(channel, src, dest, size);
    DC_InvalidateRange(dest, size);
}

cGdi::cGdi() {
    _transColor = 0;
    _mainEngineLayer = MEL_UP;
    _subEngineLayer = SEL_UP;
    _layerPitch = 0;
    _subLayerPitch = 0;
    _workMain = NULL;
    _workSub = NULL;
    _scheduleMainBackground = false;
    _scheduleMainBackdrop = false;
    _scheduleSubBackground = false;
}

cGdi::~cGdi() {
    if (NULL != _workMain) delete[] _workMain;
    if (NULL != _workSub) delete[] _workSub;
}

void cGdi::init() {
    swapLCD();
    activeFbMain();
    activeFbSub();
}

void cGdi::swapLCD(void) {
    lcdSwap();
}

void cGdi::activeFbMain(void) {
    vramSetBankE(VRAM_E_MAIN_BG);
    vramSetBankF(VRAM_F_MAIN_BG_0x06010000);
    vramSetBankG(VRAM_G_MAIN_BG_0x06014000);

    vramSetBankB(VRAM_B_MAIN_BG_0x06020000);

    vramSetBankA(VRAM_A_MAIN_SPRITE_0x06400000);

    BG_PALETTE[0] = 0x7fff;

    REG_BG2CNT = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY_1;
    REG_BG2PA = 1 << 8;  // 2 =放大倍数
    REG_BG2PD = 1 << 8;  // 2 =放大倍数
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2Y = 0;
    REG_BG2X = 0;

    REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(8) | BG_PRIORITY_2;
    REG_BG3PA = 1 << 8;  // 2 =放大倍数
    REG_BG3PD = 1 << 8;  // 2 =放大倍数
    REG_BG3PB = 0;
    REG_BG3PC = 0;
    REG_BG3Y = 0;
    REG_BG3X = 0;

    _workMain = (u16*)new u32[(256 * 192 * 3) >> 1];
    _bufferMain1 = (u16*)0x06000000;
    _bufferMain3 = (u16*)0x06020000;

    setMainEngineLayer(MEL_UP);

    fillMemory(_bufferMain1, 0x20000, 0);
    fillMemory(_bufferMain3, 0x20000, 0);

    swiWaitForVBlank();  // remove tearing at bottop screen
    videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE |
                 DISPLAY_SPR_1D_BMP_SIZE_128 | DISPLAY_SPR_1D_BMP);

    cSprite::sysMainInit();

    _mainSprites.clear();
    for (int i = 0; i < 3; i++) {
        for (int k = 0; k < 4; k++) {
            int id = i * 4 + k;
            _mainSprites.emplace_back(id, true);
            cSprite* sprite = &_mainSprites.back();
            sprite->setSize(SS_SIZE_64);
            sprite->setPriority(3);
            sprite->setPosition(k * 64, i * 64);
            sprite->show();
        }
    }

    fillMemory(_mainSprites[0].buffer(), SCREEN_WIDTH * SCREEN_HEIGHT * 2, 0xffffffff);

    _scheduleMainBackground = true;
}

void cGdi::activeFbSub(void) {
    vramSetBankC(VRAM_C_SUB_BG_0x06200000);  // 128k
    vramSetBankD(VRAM_D_SUB_SPRITE);

    BG_PALETTE_SUB[0] = 0xffff;

    REG_BG2CNT_SUB = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY_1;
    REG_BG2PA_SUB = 1 << 8;
    REG_BG2PD_SUB = 1 << 8;
    REG_BG2PB_SUB = 0;
    REG_BG2PC_SUB = 0;
    REG_BG2Y_SUB = 0;
    REG_BG2X_SUB = 0;

    _workSub = (u16*)new u32[(256 * 192 * 2) >> 1];
    _bufferSub1 = (u16*)0x06200000;

    fillMemory(_workSub, 0x18000, 0x0);
    fillMemory(_bufferSub1, 0x18000, 0x0);

    swiWaitForVBlank();
    videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_BMP_SIZE_128 | DISPLAY_SPR_1D_BMP);

    cSprite::sysSubInit();

    _subSprites.clear();
    for (int i = 0; i < 3; i++) {
        for (int k = 0; k < 4; k++) {
            int id = i * 4 + k;
            _subSprites.emplace_back(id, false);
            cSprite* sprite = &_subSprites.back();
            sprite->setSize(SS_SIZE_64);
            sprite->setPriority(2);
            sprite->setPosition(k * 64, i * 64);
            sprite->show();
        }
    }

    fillMemory(_subSprites[0].buffer(), SCREEN_WIDTH * SCREEN_HEIGHT * 2, 0xffffffff);

    _scheduleSubBackground = true;
}

void cGdi::setScreenTransparency(u16 value, GRAPHICS_ENGINE engine) {
    if (value > 100) {
        value = 0;
    }

    u16 topLvl = value / 6;
    u16 downLvl = 16 - topLvl;

    if (engine == GE_SUB) {
        REG_MASTER_BRIGHT_SUB = BIT(14) | downLvl;

        return;
    }

    // REG_BLDCNT = BLEND_ALPHA | 
    //          BLEND_SRC_SPRITE | 
    //          BLEND_SRC_BG2 | 
    //          BLEND_SRC_BG3 | 
    //          BLEND_DST_BACKDROP;
    // REG_BLDY = BLDY_EVY(0);
    // REG_BLDALPHA = topLvl | (downLvl << 8);

    REG_MASTER_BRIGHT = BIT(14) | downLvl;
}

static inline void putScreenPixel(u16* buffer, s16 x, s16 y, u16 color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;
    }

    buffer[y * 256 + x] = color;
}

void cGdi::drawPixel(u8 x, u8 y, GRAPHICS_ENGINE engine) {
    u16* buffer = engine == GE_MAIN ? _workMain + _layerPitch : _workSub + _subLayerPitch;
    u16 color = engine == GE_MAIN ? _penColor : _penColorSub;

    putScreenPixel(buffer, x, y, color);
}

void cGdi::drawLine(s16 x1, s16 y1, s16 x2, s16 y2, GRAPHICS_ENGINE engine) {
    if ((x1 == x2) && (y1 == y2)) return;

    u16* buffer = engine == GE_MAIN ? _workMain + _layerPitch : _workSub + _subLayerPitch;
    u16 color = engine == GE_MAIN ? _penColor : _penColorSub;

    if (x1 == x2) {
        int ys, ye;
        if (y1 < y2) {
            ys = y1;
            ye = y2 - 1;
        } else {
            ys = y2 + 1;
            ye = y1;
        }
        for (int py = ys; py <= ye; py++) {
            putScreenPixel(buffer, x1, py, color);
        }
        return;
    }

    if (y1 == y2) {
        int xs, xe;
        if (x1 < x2) {
            xs = x1;
            xe = x2 - 1;
        } else {
            xs = x2 + 1;
            xe = x1;
        }
        if (GE_MAIN == engine)
            fillRect(_penColor, _penColor, xs, y1, xe - xs + 1, 1, engine);
        else
            fillRect(_penColorSub, _penColorSub, xs, y1, xe - xs + 1, 1, engine);
        return;
    }

    if (abs(x2 - x1) > abs(y2 - y1)) {
        int px = 0;
        float py = 0;
        int xe = x2 - x1;
        float ye = y2 - y1;
        int xv;
        float yv;

        if (0 < xe) {
            xv = 1;
        } else {
            xv = -1;
        }
        yv = ye / abs(xe);

        while (px != xe) {
            putScreenPixel(buffer, x1 + px, y1 + (int)py, color);
            px += xv;
            py += yv;
        }
        return;
    } else {
        float px = 0;
        int py = 0;
        float xe = x2 - x1;
        int ye = y2 - y1;
        float xv;
        int yv;

        xv = xe / abs(ye);
        if (0 < ye) {
            yv = 1;
        } else {
            yv = -1;
        }

        while (py != ye) {
            putScreenPixel(buffer, x1 + (int)px, y1 + py, color);
            px += xv;
            py += yv;
        }
        return;
    }
}

void cGdi::drawRadiusLine(s16 sx, s16 sy, u16 width, u16 length, s16 degrees, u16 color, GRAPHICS_ENGINE engine) {
    static const double PI = 3.14159265358979323846;

    if (length <= width || width <= 1) {
        return;
    }

    length -= width;

    color = color | BIT(15);

    u16* buffer = engine == GE_MAIN ? _workMain + _layerPitch : _workSub + _subLayerPitch;

    degrees = (90 - degrees) % 360;
    double radius = degrees * PI / 180.0;

    s16 dx = sx + static_cast<s16>(length * std::cos(radius));
    s16 dy = sy - static_cast<s16>(length * std::sin(radius));

    int dx_abs = abs(dx - sx);
    int dy_abs = abs(dy - sy);

    int step_x = (sx < dx) ? 1 : -1;
    int step_y = (sy < dy) ? 1 : -1;

    int err = dx_abs - dy_abs;

    int xDir = -1;
    int yDir = -1;

    if (dx > sx || (dx >= sx && dy < sy)) {
        xDir = 1;
    }

    if (dy > sy || (dy >= sy && dx < sx)) {
        yDir = 1;
    }

    int x = sx;
    int y = sy;

    while (true) {
        for (int i = 0; i < width; i++) {
            for (int k = 0; k < width; k++) {
                putScreenPixel(buffer, x + xDir * i, y + yDir * k, color);
            }
        }

        if (x == dx && y == dy) break;

        int e2 = 2 * err;

        if (e2 > -dy_abs) {
            err -= dy_abs;
            x += step_x;
        }

        if (e2 < dx_abs) {
            err += dx_abs;
            y += step_y;
        }
    }
}

void cGdi::frameRect(s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine) {
    frameRect(x, y, w, h, 1, engine);
}

void cGdi::frameRect(s16 x, s16 y, u16 w, u16 h, u16 thickness, GRAPHICS_ENGINE engine) {
    if (thickness == 0 || w == 0 || h == 0) {
        return;
    }

    u16 color = getPenColor(engine);
    if (thickness * 2 >= w || thickness * 2 >= h) {
        fillRect(color, color, x, y, w, h, engine);
        return;
    }

    fillRect(color, color, x, y, w, thickness, engine);
    fillRect(color, color, x, y + h - thickness, w, thickness, engine);
    fillRect(color, color, x, y + thickness, thickness, h - 2 * thickness, engine);
    fillRect(color, color, x + w - thickness, y + thickness, thickness, h - 2 * thickness, engine);
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cGdi::fillRect)(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine) {
    s16 widthOffset = 0;
    if (x < 0) {
        widthOffset = x;
        x = 0;
    }

    if (x + w + widthOffset > SCREEN_WIDTH) {
        widthOffset = SCREEN_WIDTH - x - (s16)w;
    }

    if ((s16)w + widthOffset <= 0 || x >= SCREEN_WIDTH) {
        logger().info("Finish fill rect on width.");
        return;
    }

    s16 heightOffset = 0;
    if (y < 0) {
        heightOffset = y;
        y = 0;
    }

    if (y + h + heightOffset > SCREEN_HEIGHT) {
        heightOffset = SCREEN_HEIGHT - y - (s16)h;
    }

    if ((s16)h + heightOffset <= 0 || y >= SCREEN_HEIGHT) {
        logger().info("Finish fill rect on height.");
        return;
    }

    w += widthOffset;
    h += heightOffset;
 
    u32 color = ((u32)color1 << 16) | color2;
    u32 altColor = ((u32)color2 << 16) | color1;
    u16* pDest = GE_MAIN == engine ?
        _workMain + (y << 8) + x + _layerPitch :
        _workSub + _subLayerPitch + (y << 8) + x;

    bool aligned = !(x & 1);
    bool remains = (x ^ w) & 1;
    u16 destInc = 256 - w;
    u16 halfWidth = (w - (aligned ? 0 : 1)) >> 1;

    for (u32 i = 0; i < h; i++) {
        u32 source = (i & 1) ? color : altColor;
        if (!aligned) {
            *pDest = (u16)(source & 0x0000ffff);
            pDest++;
        }
        
        if (halfWidth == 1) {
            *(u32*)(pDest) = source;
        } else if (halfWidth == 2) {
            *(u32*)(pDest) = source;
            *(u32*)(pDest + 2) = source;
        } else if (halfWidth > 16) {
            swiFastCopy(&source, pDest, COPY_MODE_WORD | COPY_MODE_FILL | halfWidth);
        } else if (halfWidth > 0) {
            swiCopy(&source, pDest, COPY_MODE_WORD | COPY_MODE_FILL | halfWidth);
        }

        pDest += halfWidth << 1;

        if (remains) {
            *pDest++ = (u16)((source & 0xffff0000) >> 16);
        }

        pDest += destInc;
    }
}

u16 cGdi::blendColors(u16 color, u16 dest, u16 src, u16 opacity) {
    if (opacity >= 100) {
        return color;
    }

    if ((dest & 0x8000) == 0) {
        dest = src;
    }

    if (color == 0 || opacity <= 0) {
        return dest;
    }

    const u32 alpha  = (static_cast<u32>(opacity) * 655u) >> 11u;
    const u32 invAlpha = 32u - alpha;

    const u32 rb = ((static_cast<u32>(color) & 0x7c1fu) * alpha
                  + (static_cast<u32>(dest)   & 0x7c1fu) * invAlpha) & 0xf83e0u;
    const u32 g  = ((static_cast<u32>(color) & 0x3e0u)  * alpha
                  + (static_cast<u32>(dest)   & 0x3e0u)  * invAlpha) & 0x7c00u;

    return static_cast<u16>(((rb | g) >> 5u) | BIT(15));
}

u32 cGdi::blendColors32(u32 color, u32 dest, u32 src, u16 opacity) {
    if (opacity >= 100) {
        return color;
    }

    if ((dest & 0x8000) == 0) {
        dest = (dest & 0xffff0000) | (src & 0xffff);
    }

    if ((dest & 0x80000000) == 0) {
        dest = (dest & 0xffff) | (src & 0xffff0000);
    }

    if (color == 0 || opacity <= 0) {
        return dest;
    }

    const u32 alpha = (static_cast<u32>(opacity) * 655u) >> 11u;
    const u32 invAlpha = 32u - alpha;

    const u32 d1 = dest & 0xFFFFu;
    const u32 d1_exp = (d1 | (d1 << 16u)) & 0x03E07C1Fu;
    
    const u32 d2 = dest >> 16u;
    const u32 d2_exp = (d2 | (d2 << 16u)) & 0x03E07C1Fu;

    const u32 blend1 = (color * alpha + d1_exp * invAlpha) >> 5u;
    const u32 blend2 = (color * alpha + d2_exp * invAlpha) >> 5u;

    const u32 p1_out = (blend1 & 0x7C1Fu) | ((blend1 >> 16u) & 0x03E0u) | 0x8000u;
    const u32 p2_out = (blend2 & 0x7C1Fu) | ((blend2 >> 16u) & 0x03E0u) | 0x8000u;

    return p1_out | (p2_out << 16u);
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cGdi::fillRectBlend)(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine, u16 opacity) {
    if (opacity <= 0) {
        return;
    }

    if (opacity >= 100) {
        fillRect(color1, color2, x, y, w, h, engine);
        return;
    }

    if (color1 == 0 || color2 == 0) {
        return;
    }

    u16* pSrc = ((GE_MAIN == engine) ? _bufferMain3 : _workSub) + (y << 8) + x;
    u16* pDest = ((GE_MAIN == engine) ? (_workMain + _layerPitch) : (_workSub + _subLayerPitch)) + (y << 8) + x;

    bool aligned = !(x & 1);
    bool remains = (x ^ w) & 1;
    u16 halfWidth = (w - (aligned ? 0 : 1)) >> 1;
    u32 destInc = 256 - w;

    for (u32 i = 0; i < h; i++) {
        if (!aligned) {
            u16 color = (i & 1) ? color2 : color1;
            *pDest = blendColors(color, *pDest, *pSrc, opacity);
            pDest++;
            pSrc++;
        }

        u32 color32 = (i & 1) ? (((u32)color2 << 16) | color1) : (((u32)color1 << 16) | color2);
        for (u16 j = 0; j < halfWidth; j++) {
            *(u32*)pDest = blendColors32(color32, *(u32*)pDest, *(u32*)pSrc, opacity);
            pDest+=2;
            pSrc+=2;
        }

        if (remains) {
            u16 color = (i & 1) ? color1 : color2;
            *pDest = blendColors(color, *pDest, *pSrc, opacity);
            pDest++;
            pSrc++;
        }

        pDest += destInc;
        pSrc += destInc;
    }
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cGdi::bitMainBackground)(const void* src) {
    u16* pSrc = (u16*)src;

    u32 size = 256 * 192 * 2;
    DC_FlushRange(src, size);

    for (int i = 0; i < 3; i++) {
        for (int k = 0; k < 4; k++) {
            int id = i * 4 + k;
            cSprite* sprite = &_mainSprites[id];
            
            for (int l = 0; l < 64; l++) {
                u32* source = (u32*)(pSrc + (i * 64 * SCREEN_WIDTH) + (l * SCREEN_WIDTH) + (k * 64));
                u32* destination = (u32*)(sprite->buffer() + (l * 64));
                // swiFastCopy(source, destination, COPY_MODE_WORD | 32);
                dmaCopyWords(3, source, destination, 128);
            }
        }
    }

    DC_InvalidateRange(_mainSprites[0].buffer(), size);
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cGdi::bitSubBackground)(const void* src) {
    u16* pSrc = (u16*)src;

    u32 size = 256 * 192 * 2;
    DC_FlushRange(src, size);

    for (int i = 0; i < 3; i++) {
        for (int k = 0; k < 4; k++) {
            int id = i * 4 + k;
            cSprite* sprite = &_subSprites[id];
            
            for (int l = 0; l < 64; l++) {
                u32* source = (u32*)(pSrc + (i * 64 * SCREEN_WIDTH) + (l * SCREEN_WIDTH) + (k * 64));
                u32* destination = (u32*)(sprite->buffer() + (l * 64));
                // swiFastCopy(source, destination, COPY_MODE_WORD | 32);
                dmaCopyWords(3, source, destination, 128);
            }
        }
    }

    DC_InvalidateRange(_subSprites[0].buffer(), size);
}

void cGdi::bitBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine) {
    bitBlt(src, destX, destY, destW, destH, 1, engine);
}

void cGdi::bitBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, u16 repeats, GRAPHICS_ENGINE engine) {
    bitBlt(src, destW, destH, destX, destY, destW, destH, repeats, engine);
}

void cGdi::bitBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine) {
    bitBlt(src, srcW, srcH, destX, destY, destW, destH, 1, engine);
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cGdi::bitBlt)(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH, u16 repeats, GRAPHICS_ENGINE engine) {
    u16* pSrc = (u16*)src;
    u16* pDest = (engine == GE_MAIN) ? 
                    (_workMain + _layerPitch) : 
                    (_workSub + _subLayerPitch);

    if (destW > srcW) destW = srcW;
    if (destH > srcH) destH = srcH;

    if (destX + destW >= SCREEN_WIDTH) {
        s16 diff = destX + (u16)destW - SCREEN_WIDTH;
        destW -= (u16)diff;
    }

    if (destY + destH >= SCREEN_HEIGHT) {
        s16 diff = destY + (u16)destH - SCREEN_HEIGHT;
        destH -= (u16)diff;
    }

    u16 srcOffsetX = 0;
    if (destX < 0) {
        srcOffsetX = (u16)(-1 * destX);
        destW -= srcOffsetX;
        destX = 0;
    }

    u16 srcOffsetY = 0;
    if (destY < 0) {
        srcOffsetY = (u16)(-1 * destY);
        destH -= srcOffsetY;
        destY = 0;
    }

    if (destW <= 0 || destH <= 0) {
        return;
    }

    repeats = std::min(repeats, (u16)((SCREEN_WIDTH - destX) / destW));

    u16 pitchPixel = (destW + (destW & 1));
    u16 halfPitch = pitchPixel >> 1;
    bool aligned = !(destX & 1) && !(srcOffsetX & 1);
    bool even = !(destW & 1);

    u16 temp1 = 0;
    u16 temp2 = 0;
    u32 temp3 = 0;

    pDest += (destY * SCREEN_WIDTH) + destX;
    pSrc += srcOffsetX + (pitchPixel * srcOffsetY);

    for (u16 i = 0; i < destH; i++) {
        switch (destW) {
          case 1:
            temp1 = *pSrc;
            break;
          case 2:
            temp1 = *pSrc;
            temp2 = *(pSrc + 1);
            temp3 = aligned ? ((u32)temp1 << 16) | temp2 : ((u32)temp2 << 16) | temp1;
            break;
          default:
            break;
        }

        for (u16 j = 0; j < repeats; j++) {
            if (destW == 1) {
                *(pDest + j) = temp1;
            } else if (aligned && destW == 2) {
                swiFastCopy(&temp3, pDest, COPY_MODE_WORD | COPY_MODE_FILL | repeats);
                break;
            } else if (destW == 2) {
                *(pDest) = temp1;
                swiFastCopy(&temp3, pDest + 1, COPY_MODE_WORD | COPY_MODE_FILL | (repeats - 1));
                *(pDest -1 + (2 * repeats)) = temp2;
                break;
            } else if (aligned && even && (destW != 0) && halfPitch <= 16) {
                swiCopy(pSrc, pDest + (j * destW), COPY_MODE_WORD | halfPitch);
            } else if (aligned && even && (destW != 0)) {
                swiFastCopy(pSrc, pDest + (j * destW), COPY_MODE_WORD | halfPitch);
            } else {
                swiCopy(pSrc, pDest + (j * destW), COPY_MODE_COPY | destW);
            }
        }
        
        pSrc += pitchPixel;
        pDest += SCREEN_WIDTH;
    }
}

void cGdi::maskBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine) {
    return maskBlt(src, destW, destH, destX, destY, destW, destH, engine, 0);
}

void cGdi::maskBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine, u16 color) {
    return maskBlt(src, destW, destH, destX, destY, destW, destH, engine, color);
}

void cGdi::maskBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH,
                   GRAPHICS_ENGINE engine) {
    return maskBlt(src, srcW, srcH, destX, destY, destW, destH, engine, 0);
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cGdi::maskBlt)(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH,
                   GRAPHICS_ENGINE engine, u16 color) {
    u16* pSrc = (u16*)src;
    u16* pDest = GE_MAIN == engine ?
        _workMain + _layerPitch :
        _workSub + _subLayerPitch;

    if (destW > srcW) destW = srcW;
    if (destH > srcH) destH = srcH;

    if (destX + destW >= SCREEN_WIDTH) {
        s16 diff = destX + destW - SCREEN_WIDTH;
        destW -= diff;
    }

    if (destY + destH >= SCREEN_HEIGHT) {
        s16 diff = destY + destH - SCREEN_HEIGHT;
        destH -= diff;
    }

    u16 srcOffsetX = 0;
    if (destX < 0) {
        srcOffsetX = (u16)(-1 * destX);
        destW -= srcOffsetX;
        destX = 0;
    }

    u16 srcOffsetY = 0;
    if (destY < 0) {
        srcOffsetY = (u16)(-1 * destY);
        destH -= srcOffsetY;
        destY = 0;
    }

    if (destW <= 0 || destH <= 0) {
        return;
    }

    u16 pitch = srcW + (srcW & 1);
    bool colorOverride = color != 0;

    u16 srcOffset = 0;
    u16 destOffset = 0;
    u16 start = 0;
    u16 length = 0;

    for (u16 i = 0; i < destH; i++) {
        srcOffset = srcOffsetX + (i + srcOffsetY) * pitch;
        destOffset = destX + (i + destY) * SCREEN_WIDTH;
        start = 0;
        length = 0;
        for (u16 k = 0; k < destW; k++) {
            u16 srcPixel = *(pSrc + srcOffset + k);
            if (srcPixel & 0x8000) {
                if (length == 0) {
                    start = k;
                }

                length++;

                if (k == 0) {
                    u16 lastPixel = *(pSrc + srcOffset + destW - 1);
                    if (lastPixel & 0x8000) {
                        length = destW;
                        break;
                    }
                }

                continue;
            }

            if (length == 0) {
                continue;
            } else if (colorOverride) {
                for (u16 l = 0; l < length; l++) {
                    *(pDest + destOffset + start + l) = color;
                }
            } else if (length == 1) {
                *(pDest + destOffset + start) = *(pSrc + srcOffset + start);
            } else if (length <= 4) {
                memcpy(pDest + destOffset + start, pSrc + srcOffset + start, length * sizeof(u16));
            } else if (((srcOffset + start) & 1) || ((destOffset + start) & 1) || (length & 1)) {
                swiCopy(pSrc + srcOffset + start, pDest + destOffset + start, COPY_MODE_COPY | length);
            } else if (length <= 32) {
                swiCopy(pSrc + srcOffset + start, pDest + destOffset + start, COPY_MODE_WORD | length >> 1);
            } else {
                swiFastCopy(pSrc + srcOffset + start, pDest + destOffset + start, COPY_MODE_WORD | length >> 1);
            }
            
            length = 0;
        }

        if (length == 0) {
            continue;
        } else if (colorOverride) {
            for (u16 l = 0; l < length; l++) {
                *(pDest + destOffset + start + l) = color;
            }
        } else if (length == 1) {
            *(pDest + destOffset + start) = *(pSrc + srcOffset + start);
        } else if (length <= 4) {
            memcpy(pDest + destOffset + start, pSrc + srcOffset + start, length * sizeof(u16));
        } else if (((srcOffset + start) & 1) || ((destOffset + start) & 1) || (length & 1)) {
            swiCopy(pSrc + srcOffset + start, pDest + destOffset + start, COPY_MODE_COPY | length);
        } else if (length <= 32) {
                swiCopy(pSrc + srcOffset + start, pDest + destOffset + start, COPY_MODE_WORD | length >> 1);
        } else {
            swiFastCopy(pSrc + srcOffset + start, pDest + destOffset + start, COPY_MODE_WORD | COPY_MODE_COPY | length >> 1);
        }
    }
}

s16 cGdi::textOut(s16 x, s16 y, const char* text, GRAPHICS_ENGINE engine) {
    return textOutRect(x, y, 256, 192, text, engine, font());
}

s16  cGdi::textOut(s16 x, s16 y, const char* text, GRAPHICS_ENGINE engine, const cFont& textFont) {
    return textOutRect(x, y, 256, 192, text, engine, textFont);
}

s16  cGdi::textOutRect(s16 x, s16 y, u16 w, u16 h, const char* text, GRAPHICS_ENGINE engine) {
    return textOutRect(x, y, w, h, text, engine, font());
}

ARM_CODE LIBNDS_NOINLINE
s16 ITCM_FUNC(cGdi::textOutRect)(s16 x, s16 y, u16 w, u16 h, const char* text, GRAPHICS_ENGINE engine, const cFont& textFont) {
    u8 fontHeight = textFont.GetHeight();
    u8 fontDescend = textFont.GetDescend();
    u16* pDest = GE_MAIN == engine ? _workMain + _layerPitch : _workSub + _subLayerPitch;
    u16 color = GE_MAIN == engine ? _penColor : _penColorSub;

    const s16 originX = x, limitY = y + h - fontHeight;
    while (*text) {
        if ('\r' == *text || '\n' == *text) {
            y += (fontHeight + fontDescend);
            x = originX;
            ++text;
            if (y > limitY) break;
        } else {
            u32 ww, add;
            textFont.Info(text, &ww, &add);
            if (x < 0 || x + (s16)ww <= std::min(originX + w, SCREEN_WIDTH)) {
                textFont.Draw(pDest, x, y, (const u8*)text, color, SCREEN_WIDTH, SCREEN_HEIGHT);
            }
            text += add;
            x += ww;
        }
    }

    return x;
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cGdi::pushMainBackground)() {
    _scheduleMainBackground = true;
    bitMainBackground(_workMain + (MEL_DOWN * 256 * 192));
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cGdi::pushSubBackground)() {
    _scheduleSubBackground = true;
    bitSubBackground(_workSub + (SEL_DOWN * 256 * 192));
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cGdi::present)() {
    if (_scheduleSubBackground) {
        oamUpdate(&oamSub);
        swiWaitForVBlank();
        swiWaitForVBlank();
    }

    dmaCopyWordsGdi(2, (void*)_workSub, (void*)_bufferSub1, 256 * 192 * 2);
    
    if (_scheduleMainBackground) {
        oamUpdate(&oamMain);
    }

    if (_scheduleMainBackground || _scheduleMainBackdrop) {
        swiWaitForVBlank();
    }

    if (_scheduleMainBackdrop) {
        dmaCopyWordsGdi(3, _workMain + (MEL_MIDDLE * SCREEN_WIDTH * SCREEN_HEIGHT), _bufferMain3, 256 * 192 * 2);
    }

    dmaCopyWordsGdi(3, _workMain, _bufferMain1, 256 * 192 * 2);

    fillMemory((void*)(_workMain), SCREEN_WIDTH * SCREEN_HEIGHT * 2, 0);
    fillMemory((void*)(_workSub), SCREEN_WIDTH * SCREEN_HEIGHT * 2, 0);

    if (_scheduleMainBackdrop) {
        fillMemory((void*)(_workMain + (MEL_MIDDLE * SCREEN_WIDTH * SCREEN_HEIGHT)), SCREEN_WIDTH * SCREEN_HEIGHT * 2, 0);
    }

    _scheduleMainBackground = false;
    _scheduleMainBackdrop = false;
    _scheduleSubBackground = false;
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cGdi::presentMain)() {
    if (_scheduleMainBackground) {
        oamUpdate(&oamMain);
    }

    if (_scheduleMainBackground || _scheduleMainBackdrop) {
        swiWaitForVBlank();
    }

    if (_scheduleMainBackdrop) {
        dmaCopyWordsGdi(3, _workMain + (MEL_MIDDLE * SCREEN_WIDTH * SCREEN_HEIGHT), _bufferMain3, 256 * 192 * 2);
    }

    dmaCopyWordsGdi(3, _workMain, _bufferMain1, 256 * 192 * 2);

    fillMemory((void*)(_workMain), SCREEN_WIDTH * SCREEN_HEIGHT * 2, 0);

    if (_scheduleMainBackdrop) {
        fillMemory((void*)(_workMain + (MEL_MIDDLE * SCREEN_WIDTH * SCREEN_HEIGHT)), SCREEN_WIDTH * SCREEN_HEIGHT * 2, 0);
    }

    _scheduleMainBackground = false;
    _scheduleMainBackdrop = false;
}
