#include "vfxmanager.h"
#include "globalsettings.h"
#include "tick_sound_bin.h"
#include "select_sound_bin.h"
#include "click_sound_bin.h"
#include "close_sound_bin.h"
#include "up_sound_bin.h"
#include "save_sound_bin.h"
#include "next_sound_bin.h"

VfxManager::VfxManager() {
    _init = false;
    _enabled = false;
}

void VfxManager::init() {
    _entries.push_back(VfxEntry(VFX_EFFECT::TICK, (u8*)tick_sound_bin, tick_sound_bin_size, 20));
    _entries.push_back(VfxEntry(VFX_EFFECT::SELECT, (u8*)select_sound_bin, select_sound_bin_size, 66));
    _entries.push_back(VfxEntry(VFX_EFFECT::CLICK, (u8*)click_sound_bin, click_sound_bin_size, 10));
    _entries.push_back(VfxEntry(VFX_EFFECT::CLOSE, (u8*)close_sound_bin, close_sound_bin_size, 10));
    _entries.push_back(VfxEntry(VFX_EFFECT::UP, (u8*)up_sound_bin, up_sound_bin_size, 10));
    _entries.push_back(VfxEntry(VFX_EFFECT::SAVE, (u8*)save_sound_bin, save_sound_bin_size, 25));
    _entries.push_back(VfxEntry(VFX_EFFECT::NEXT, (u8*)next_sound_bin, next_sound_bin_size, 10));
    _init = true;
    _enabled = true;
}

void VfxManager::enable() {
    _enabled = true;
}

void VfxManager::disable() {
    _enabled = false;;
}

void VfxManager::update() {
    for (VfxManager::VfxEntry& entry : _entries) {
        if (entry.ticks > 0) {
            entry.ticks--;
        } else {
            entry.channel = UINT16_MAX;
        }
    }
}

bool VfxManager::busy() const {
    for (VfxManager::VfxEntry entry : _entries) {
        if (entry.ticks > 0) {
            return true;
        }
    }

    return false;
}

void VfxManager::playEffect(VFX_EFFECT e) {
    playEffect(e, 127);
}

void VfxManager::playEffect(VFX_EFFECT e, u16 v) {
    if (!_init || !_enabled || !gs().sound) {
        return;
    }

    VfxEntry& entry = getEntry(e);
    if (entry.effect == VFX_EFFECT::NONE) {
        return;
    }

    soundKill(entry.channel);

    entry.ticks = entry.ticksMax;
    entry.channel = soundPlaySample(entry.sound, SoundFormat_16Bit, entry.length, 32000, v, 64, false, 0);
}

VfxManager::VfxEntry& VfxManager::getEntry(VFX_EFFECT e) {
    for (VfxManager::VfxEntry& entry : _entries) {
        if (entry.effect == e) {
            return entry;
        }
    }

    return _default;
}