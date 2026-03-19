#include "ticksound.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "systemdetails.h"
#include "datetime.h"
#include "globalsettings.h"

cTickSound::cTickSound() {
    _rawData = NULL;
    _dataSize = 0;
    _pcmStart = NULL;
    _sampleRate = 0;
    _soundFormat = 0;
    _checkpoint = 0;
    _soundId = 0;
    _schedule = 0;
}

cTickSound::~cTickSound() {
    unload();
}

void cTickSound::unload() {
    if (_rawData) {
        free(_rawData);
        _rawData = NULL;
    }

    if (_pcmStart) {
        free(_pcmStart);
        _pcmStart = NULL;
    }

    _dataSize = 0;
}

bool cTickSound::load(std::string filepath) {
    FILE* file = fopen(filepath.c_str(), "rb");
    if (!file) return false;

    fseek(file, 0, SEEK_END);
    u32 fileSize = ftell(file);
    rewind(file);

    if (fileSize < 44) {
        fclose(file);
        return false;
    }

    unload();

    _rawData = (u8*)memalign(32, fileSize);
    if (!_rawData) {
        fclose(file);
        return false;
    }

    fread(_rawData, 1, fileSize, file);
    fclose(file);

    _sampleRate = *(u32*)(_rawData + 24);

    u16 bitsPerSample = *(u16*)(_rawData + 34);
    if (bitsPerSample == 16) {
        _soundFormat = SoundFormat_16Bit;
    } else if (bitsPerSample == 8) {
        _soundFormat = SoundFormat_8Bit;
    } else {
        unload();
        return false;
    }

    u16 audioFormat = *(u16*)(_rawData + 20);
    if (audioFormat != 1) {
        unload();
        return false;
    }

    u16 numChannels = *(u16*)(_rawData + 22);
    if (numChannels != 1) {
        unload();
        return false;
    }

    u8* ptr = _rawData + 12;
    u8* dataStart = NULL;
    u32 dataSize = 0;

    while (ptr < _rawData + fileSize) {
        u32 chunkId = *(u32*)ptr;
        u32 chunkSize = *(u32*)(ptr + 4);

        if (chunkId == 0x61746164) {
            dataStart = ptr + 8;
            dataSize = chunkSize;
            break;
        }

        ptr += 8 + chunkSize;
    }

    if (!dataStart || dataSize == 0) {
        unload();
        return false;
    }

    _pcmStart = (u8*)memalign(32, dataSize);
    if (!_pcmStart) {
        unload();
        return false;
    }

    memcpy(_pcmStart, dataStart, dataSize);
    _dataSize = dataSize;

    DC_FlushRange(_pcmStart, _dataSize);

    _checkpoint = datetime().secondsInDay();

    return true;
}

void cTickSound::play() {
    if (!sd().fifoStatus()) {
        return;
    }

    if (_pcmStart == NULL) {
        return;
    }

    if (!gs().clockSound) {
        return;
    }

    if (_schedule > 0) {
        _schedule--;
    }

    u64 now = datetime().secondsInDay();
    if ((now - _checkpoint) <= 0 && now >= _checkpoint && _schedule != 1) {
        return;
    }

    _checkpoint = now;

    soundKill(_soundId);
    _soundId = soundPlaySample(_pcmStart, (SoundFormat)_soundFormat, _dataSize, _sampleRate, 42, 64, false, 0);

    if (datetime().seconds() == 0 && _schedule == 0) {
        _schedule = 20;
    }
}