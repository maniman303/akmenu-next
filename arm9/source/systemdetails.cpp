#include <nds/arm9/dldi.h>
#include <nds.h>
#include "systemdetails.h"
#include "fifotool.h"

#define FIFO_SOUND_READY 0x1234

extern "C" {
	static volatile bool _fifoReady = false;
	static volatile int _batteryStatus = 0;

	void soundCallback(u32 msg, void *userdata) {
		u32 key = msg >> 16;
		u32 value = msg & 0xffff;

		if (key == FIFO_SOUND_READY) {
			if (!_fifoReady) {
				soundEnable();
			}

			_fifoReady = true;
		}

		if (key == MENU_MSG_BATTERY_STATE) {
			_batteryStatus = value;
		}
	}

	void registerFifoHandlers() {
		fifoSetValue32Handler(FIFO_USER_03, soundCallback, NULL);
	}
}

cSystemDetails::cSystemDetails() {
    _fifoInit = false;
}

void cSystemDetails::initArm7RegStatuses() {
	if (!_fifoInit) {
		registerFifoHandlers();
	}
    
	_fifoInit = true;
}

bool cSystemDetails::fifoStatus() {
	return _fifoReady;
}

int cSystemDetails::batteryStatus() {
	return _batteryStatus;
}