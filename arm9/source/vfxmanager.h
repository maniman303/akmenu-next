#pragma once

#include <nds.h>
#include <deque>
#include "singleton.h"
#include "tasks/workindicator.h"

enum VFX_EFFECT { NONE, TICK, SELECT, CLICK, CLOSE, UP, SAVE };

class VfxManager : public WorkIndicator {
    struct VfxEntry {
      u16 channel;
      u16 ticks;
      u16 ticksMax;
      u32 length;
      u8* sound;
      VFX_EFFECT effect;

      VfxEntry() : VfxEntry(VFX_EFFECT::NONE, NULL, 0, 0) {}
      VfxEntry(VFX_EFFECT e, u8* s, u32 l, u16 t) : channel(UINT16_MAX), ticks(0), ticksMax(t), length(l), sound(s), effect(e) {}
    };

  public:
    VfxManager();

    void init();
    void enable();
    void disable();
    void update();
    bool busy() const override;
    void playEffect(VFX_EFFECT e);
    void playEffect(VFX_EFFECT e, u16 v);

  private:
    VfxEntry& getEntry(VFX_EFFECT e);

    bool _init;
    bool _enabled;
    VfxEntry _default;
    std::deque<VfxEntry> _entries;
};

typedef t_singleton<VfxManager> VfxManager_s;
inline VfxManager& vfxManager() {
    return VfxManager_s::instance();
}