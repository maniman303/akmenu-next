/*
    bmp15.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

//�

#include <list>
#include <string>
#include "bmp15.h"
#include "dbgtool.h"

cBMP15::cBMP15() : _width(0), _height(0), _pitch(0), _buffer(NULL), _filename("") {}

cBMP15::cBMP15(u32 width, u32 height) : _width(0), _height(0), _pitch(0), _buffer(NULL), _filename("") {
    _width = width;
    _height = height;
    _pitch = (width + (width & 1)) << 1;
}

cBMP15::~cBMP15() {
    // use destroyBMP15
}

void cBMP15::freeBuffer() {
    if (_buffer == NULL) {
        return;
    }

    delete[] _buffer;
    _buffer = NULL;
}

std::string cBMP15::filename() const {
    return _filename;
}

std::string cBMP15::filename(std::string filename) {
    _filename = filename;

    return _filename;
}

cBMP15 createBMP15(u32 width, u32 height) {
    cBMP15 bmp(width, height);

    u32 pitch = bmp.pitch();  // 15bit bmp pitch 算法
    // dbg_printf( "pitch: %d bytes\n", pitch );
    // dbg_printf( "buffer %08x\n", bmp.buffer() );

    u32 bufferSize = height * pitch;
    if (bufferSize & 3)  // 如果 bufferSize 不是按4字节对齐，就把他调整到对齐
        bufferSize += 4 - (bufferSize & 3);
    bmp._buffer = new u32[bufferSize >> 2];
    return bmp;
}

cBMP15 createBMP15(u32 width, u32 height, u32 color) {
    cBMP15 bmp = createBMP15(width, height);
    swiFastCopy((void*)(&color), bmp.buffer(), ((bmp.height() * bmp.pitch()) >> 2) | COPY_MODE_WORD | COPY_MODE_FILL);

    return bmp;
}

typedef std::list<cBMP15> str_bmp_list;
static str_bmp_list _bmpPool;

cBMP15 createBMP15FromFile(const std::string& filename) {
    // dbg_printf( "createBMP15FromFile (%s)\n", filename );

    str_bmp_list::iterator it;
    for (it = _bmpPool.begin(); it != _bmpPool.end(); ++it) {
        if (it->filename() == filename) {
            return *it;
        }
    }

    FILE* f = fopen(filename.c_str(), "rb");
    if (NULL == f) {
        dbg_printf("(%s) file does not exist\n", filename.c_str());
        return cBMP15();
    }

    fseek(f, 0, SEEK_END);
    int fileSize = ftell(f);

    if (-1 == fileSize) {
        fclose(f);
        return cBMP15();
    }

    u16 bmMark = 0;
    fseek(f, 0, SEEK_SET);
    fread(&bmMark, 1, 2, f);
    if (bmMark != 0x4d42) {  // 'B' 'M' header
        dbg_printf("not a bmp file\n");
        fclose(f);
        return cBMP15();
    }

    u32 width = 0;
    u32 height = 0;
    fseek(f, 0x12, SEEK_SET);
    fread(&width, 1, 4, f);
    fseek(f, 0x16, SEEK_SET);
    fread(&height, 1, 4, f);

    cBMP15 bmp = createBMP15(width, height);

    u32 bmpDataOffset = 0;
    fseek(f, 0x0a, SEEK_SET);
    fread(&bmpDataOffset, 1, 4, f);

    long position = bmpDataOffset;
    fseek(f, position, SEEK_SET);
    u16* pbuffer = ((u16*)bmp.buffer()) + (bmp.pitch() >> 1) * height - (bmp.pitch() >> 1);

    for (u32 i = 0; i < height; ++i) {
        fread(pbuffer, 1, bmp.pitch(), f);
        position += bmp.pitch();
        pbuffer -= bmp.pitch() >> 1;
        fseek(f, position, SEEK_SET);
    }
    fclose(f);

    pbuffer = (u16*)bmp.buffer();
    for (u32 i = 0; i < height; ++i) {
        for (u32 j = 0; j < (bmp.pitch() >> 1); ++j) {
            u16 pixelColor = pbuffer[i * (bmp.pitch() >> 1) + j];
            pixelColor = ((pixelColor & 0x7C00) >> 10) | ((pixelColor & 0x03E0)) |
                         ((pixelColor & 0x1F) << 10);
            pbuffer[i * (bmp.pitch() >> 1) + j] = pixelColor | (pixelColor ? BIT(15) : 0);
            // dbg_printf("%d               %d\n", j, i );
        }
    }

    bmp.filename(filename);
    _bmpPool.push_back(bmp);

    // dbg_printf( "load bmp success, %08x\n", bmp.buffer() );
    return bmp;
}

void destroyBMP15(cBMP15& bmp) {
    _bmpPool.remove_if([&](const cBMP15& testItem) {
        return testItem.filename() == bmp.filename();
    });

    bmp.freeBuffer();
}