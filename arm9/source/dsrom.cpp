/*
    dsrom.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "dsrom.h"
#include "dbgtool.h"
#include "gamecode.h"
#include "gbarom_banner_bin.h"
#include "icon_bg_bin.h"
#include "icons.h"
#include "nds_banner_bin.h"
#include "unknown_nds_banner_bin.h"
#include "unicode.h"
#include "logger.h"
#include "fsmngr.h"
#include "../../share/memtool.h"

static const u32 ROM_READ_SIZE = 0x200;
static u8 sRomReadBuf[ROM_READ_SIZE] __attribute__((aligned(4)));

DSRomInfo::DSRomInfo() {
    _isDSRom = EFalse;
    _isDSiWare = EFalse;
    _isHomebrew = EFalse;
    _isModernHomebrew = EFalse;
    _isGbaRom = EFalse;
    _romVersion = 0;
    _buffer = NULL;
    _lastSize = false;
    memset(&_banner, 0, sizeof(_banner));
    memset(&_saveInfo, 0, sizeof(_saveInfo));
}

DSRomInfo::~DSRomInfo() { }

DSRomInfo& DSRomInfo::operator=(const DSRomInfo& src) {
    swiCopy(&src._banner, &_banner, COPY_MODE_WORD | (sizeof(_banner) / 4));
    swiCopy(&src._saveInfo, &_saveInfo, COPY_MODE_WORD | (sizeof(_saveInfo) / 4));
    _isDSRom = src._isDSRom;
    _isHomebrew = src._isHomebrew;
    _isModernHomebrew = src._isModernHomebrew;
    _isGbaRom = src._isGbaRom;
    _fileName = src._fileName;
    _romVersion = src._romVersion;
    _buffer = src._buffer;
    return *this;
}

bool DSRomInfo::loadDSRomInfo(const std::string& filename, bool loadBanner) {
    _isDSRom = EFalse;
    _isHomebrew = EFalse;
    _isDSiWare = EFalse;
    _isModernHomebrew = EFalse;

    FILE* f = fopen(filename.c_str(), "rb");
    if (f == NULL) {
        return false;
    }

    u32 bytesRead = (u32)fread(sRomReadBuf, 1, ROM_READ_SIZE, f);

    if (bytesRead < ROM_READ_SIZE) {
        swiCopy(unknown_nds_banner_bin, &_banner, COPY_MODE_WORD | (sizeof(_banner) / 4));
        fclose(f);
        return false;
    }

    tNDSHeader* header = (tNDSHeader*)sRomReadBuf;
    if (header->unitCode == 0x03) {
        _isDSiWare = ETrue;
    }

    ///////// ROM Header /////////
    u16 crc = swiCRC16(0xFFFF, header, 0x15E);
    if (crc != header->headerCRC16) {
        swiCopy(unknown_nds_banner_bin, &_banner, COPY_MODE_WORD | (sizeof(_banner) / 4));
        fclose(f);
        return true;
    }

    _isDSRom = ETrue;

    // Check for modern homebrew
    // Assume homebrew until proven otherwise
    _isHomebrew = ETrue;
    _isModernHomebrew = ETrue;
    u32 arm9StartSig[4] = {0};

    // "Battle/Combat of Giants: Mutant Insects" (TID: BIG) has code that is
    // run before the actual SDK boot code.
    u32 arm9SigOffset = (u32)header->arm9romOffset +
        ((strncmp(header->gameCode, "BIG", 3) == 0)
            ? 0x02000800
            : (u32)header->arm9executeAddress)
        - (u32)header->arm9destination;

    fseek(f, arm9SigOffset, SEEK_SET);
    fread(arm9StartSig, sizeof(u32), 4, f);

    // Check for Nintendo SDK style retail builds
    if ((arm9StartSig[0] == 0xE3A0C301 || (arm9StartSig[0] >= 0xEA000000 && arm9StartSig[0] < 0xEC000000)) && arm9StartSig[1] == 0xE58CC208) {
        if ((arm9StartSig[2] >= 0xEB000000 && arm9StartSig[2] < 0xEC000000) && (arm9StartSig[3] >= 0xE3A00000 && arm9StartSig[3] < 0xE3A01000)) {
            _isHomebrew = EFalse;
            _isModernHomebrew = EFalse;
        } else if (arm9StartSig[2] == 0xE1DC00B6 && arm9StartSig[3] == 0xE3500000) {
            _isHomebrew = EFalse;
            _isModernHomebrew = EFalse;
        } else if (arm9StartSig[2] == 0xEAFFFFFF && arm9StartSig[3] == 0xE1DC00B6) {
            _isHomebrew = EFalse;
            _isModernHomebrew = EFalse;
        }
    } else if (strncmp(header->gameCode, "HNA", 3) == 0) {
        // Modcrypted retail game
        _isHomebrew = EFalse;
        _isModernHomebrew = EFalse;
    }

    if (_isHomebrew) {
        if (arm9StartSig[0] == 0xE3A00301
        && arm9StartSig[1] == 0xE5800208
        && arm9StartSig[2] == 0xE3A00013
        && arm9StartSig[3] == 0xE129F000) {
            // Modern hb signature, but check some known old cases
            if ((u32)header->arm7executeAddress >= 0x037F0000 && (u32)header->arm7destination >= 0x037F0000) {
                switch (header->arm9binarySize) {
                    case 0xC9F68:  if (header->arm7binarySize == 0x12814) _isModernHomebrew = EFalse; break; // Colors! v1.1
                    case 0x1B0864: if (header->arm7binarySize == 0xDB50)  _isModernHomebrew = EFalse; break; // Mario Paint Composer DS v2
                    case 0xE78FC:  if (header->arm7binarySize == 0xF068)  _isModernHomebrew = EFalse; break; // SnowBros v2.2
                    case 0xD45C0:  if (header->arm7binarySize == 0x2B7C)  _isModernHomebrew = EFalse; break; // ikuReader v0.058
                    case 0x7A124:  if (header->arm7binarySize == 0xEED0)  _isModernHomebrew = EFalse; break; // PPSEDS r11
                    case 0x54620:  if (header->arm7binarySize == 0x1538)  _isModernHomebrew = EFalse; break; // XRoar 0.24fp3
                    case 0x2C9A8:  if (header->arm7binarySize == 0xFB98)  _isModernHomebrew = EFalse; break; // NitroGrafx v0.7
                    case 0x22AE4:  if (header->arm7binarySize == 0xA764)  _isModernHomebrew = EFalse; break; // It's 1975...
                    default: break;
                }
            }
        } else if ((header->unitCode == 0) &&
            ((memcmp(header->gameTitle, "NMP4BOOT", 8) == 0) ||
                ((u32)header->arm7executeAddress >= 0x037F0000 && (u32)header->arm7destination >= 0x037F0000))) {
            _isModernHomebrew = EFalse; // Old hb requiring DLDI
        }

        // 0x1BF is within the first 512 bytes we already read
        u8 accessControl = sRomReadBuf[0x1BF];

        if (!_isHomebrew && (header->unitCode != 0) && (accessControl & BIT(4))) {
            _isDSiWare = ETrue;
        }
    }
    
    if ((u32)(header->arm7destination) >= 0x037F8000 ||
        0x23232323 == gamecode(header->gameCode)) {  // 23->'#'
        _isHomebrew = ETrue;
    }

    ///////// saveInfo /////////
    swiCopy(header->gameTitle, _saveInfo.gameTitle, COPY_MODE_WORD | 3);
    swiCopy(header->gameCode, _saveInfo.gameCode, COPY_MODE_WORD | 1);
    _saveInfo.gameCRC = header->headerCRC16;
    saveManager().updateSaveInfoByInfo(_saveInfo);
    _romVersion = header->romversion;

    ///////// banner /////////
    // Read banner from file
    if (header->bannerOffset != 0) {
        fseek(f, header->bannerOffset, SEEK_SET);
        u32 readed = (u32)fread(&_banner, 1, 0x840, f);
        if (sizeof(tNDSBanner) != readed) {
            setBannerFromFile(fsManager().getIconPath("nds_banner.bin"), nds_banner_bin);
        } else {
            crc = swiCRC16(0xffff, _banner.icon, 0x840 - 32);
            if (crc != _banner.crc) {
                setBannerFromFile(fsManager().getIconPath("nds_banner.bin"), nds_banner_bin);
            }
        }
    } else {
        setBannerFromFile(fsManager().getIconPath("nds_banner.bin"), nds_banner_bin);
    }

    _buffer = NULL;
    fclose(f);
    f = NULL;

    return true;
}

void DSRomInfo::drawDSRomIcon(u8 x, u8 y, bool small, GRAPHICS_ENGINE engine) {
    u16 iconSize = small ? 16 : 32;
    if (_saveInfo.getIcon() == SAVE_INFO_EX_ICON_FIRMWARE) {
        gdi().maskBlt(icon_bg_bin, x, y, iconSize, iconSize, engine);
    }

    if (small != _lastSize || !_buffer) {
        _buffer = std::shared_ptr<u32[]>(new u32[small ? 16 * 8 : 32 * 16]);
        _lastSize = small;
        if (!isGbaRom() || !tryLoadGbaIcon(small)) {
            drawDSRomIconMem((u16*)_buffer.get(), small);
        }
    }

    gdi().maskBlt(_buffer.get(), x, y, iconSize, iconSize, engine);
}

void DSRomInfo::drawDSRomIconMem(u16* mem, bool small) {
    bool skiptransparent = _saveInfo.getIcon() == SAVE_INFO_EX_ICON_AS_IS;
    u16 size = small ? 16 : 32;
    fillMemory(_buffer.get(), small ? 16 * 8 : 32 * 16 * sizeof(u32), 0);
    for (int tile = 0; tile < 16; ++tile) {
        for (int pixel = 0; pixel < 32; ++pixel) {
            u8 a_byte = _banner.icon[(tile << 5) + pixel];

            int px = ((tile & 3) << 3) + ((pixel << 1) & 7);
            int py = ((tile >> 2) << 3) + (pixel >> 2);

            if (small) {
                px /= 2;
                py /= 2;
            }

            u8 idx1 = (a_byte & 0xf0) >> 4;
            u8 idx2 = (a_byte & 0x0f);

            if (skiptransparent || 0 != idx1) {
                *(mem + (py * size) + px + 1) = BIT(15) | _banner.palette[idx1];
            }

            if (skiptransparent || 0 != idx2) {
                *(mem + (py * size) + px) = BIT(15) | _banner.palette[idx2];
            }
        }
    }
}

bool DSRomInfo::drawDSRomIconBmp(const cBMP15& icon, bool small) {
    if (!icon.valid() || icon.width() != 32 || icon.height() != 32) {
        return false;
    }

    if (!small) {
        swiCopy(icon.buffer(), _buffer.get(), 32 * 32);
        return true;
    }

    for (u16 i = 0; i < 16; i++) {
        for (u16 k = 0; k < 16; k++) {
            u16* srcPos = (u16*)icon.buffer() + i * 64 + k * 2;
            u16* dstPos = (u16*)_buffer.get() + i * 16 + k;
            *dstPos = *srcPos;
        }
    }

    return true;
}

std::string DSRomInfo::getDsLocTitle() {
    if (!isDSRom()) {
        return "";
    }

    return unicode_to_local_string(banner().titles[gs().language], 128, NULL);
}

bool DSRomInfo::loadGbaRomInfo(const std::string& filename) {
    _isGbaRom = EFalse;
    FILE* gbaFile = fopen(filename.c_str(), "rb");
    if (gbaFile) {
        sGBAHeader header;
        fread(&header, 1, sizeof(header), gbaFile);
        fclose(gbaFile);
        if (header.is96h == 0x96) {
            _isGbaRom = ETrue;
            memcpy(_saveInfo.gameCode, header.gamecode, 4);
            _romVersion = header.version;
            setBannerFromFile(fsManager().getIconPath("gbarom_banner.bin"), gbarom_banner_bin);
            return true;
        }
    }
    return false;
}

void DSRomInfo::load(void) {
    if (_isDSRom == EMayBe) {
        loadDSRomInfo(_fileName, true);
    }

    if (_isGbaRom == EMayBe) {
        loadGbaRomInfo(_fileName);
    }
}

tNDSBanner& DSRomInfo::banner(void) {
    return _banner;
}

const SAVE_INFO_EX& DSRomInfo::saveInfo(void) const {
    return _saveInfo;
}

SAVE_INFO_EX& DSRomInfo::saveInfo(void) {
    return _saveInfo;
}

u8 DSRomInfo::version(void) {
    return _romVersion;
}

bool DSRomInfo::isDSRom(void) const {
    return (_isDSRom == ETrue) ? true : false;
}

bool DSRomInfo::isDSiWare(void) const {
    return (_isDSiWare == ETrue) ? true : false;
}

bool DSRomInfo::isHomebrew(void) const {
    return (_isHomebrew == ETrue) ? true : false;
}

bool DSRomInfo::isModernHomebrew(void) const {
    return (_isModernHomebrew == ETrue) ? true : false;
}

bool DSRomInfo::isGbaRom(void) const {
    return (_isGbaRom == ETrue) ? true : false;
}

bool DSRomInfo::setBannerFromFile(const std::string& path, const u8* aBanner)
{
    bool res = false;
    FILE* f = fopen(path.c_str(), "rb");
    if (f != NULL) {
        size_t read = fread(&banner(), 1, sizeof(tNDSBanner), f);
        fclose(f);
        res = read == sizeof(tNDSBanner);
    } else {
        swiCopy(aBanner, &banner(), COPY_MODE_WORD | (sizeof(tNDSBanner) / 4));
        res = true;
    }

    if (!res) {
        return false;
    }

    _buffer = NULL;

    return true;
}

bool DSRomInfo::lastSize() {
    return _lastSize;
}

bool DSRomInfo::tryLoadGbaIcon(bool size) {
    if (!isGbaRom()) {
        return false;
    }

    std::string iconFileName = _fileName;
    std::size_t dotPos = iconFileName.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false;
    }

    std::size_t slashPos = iconFileName.find_last_of("/\\");
    if (slashPos != std::string::npos && dotPos < slashPos) {
        return false;
    }

    iconFileName.replace(dotPos, std::string::npos, ".bmp");
    cBMP15 icon = createBMP15FromFile(iconFileName);

    return drawDSRomIconBmp(icon, size);
}