#include <nds/arm9/dldi.h>
#include <nds.h>
#include <cstdio>
#include "systemdetails.h"
#include "fifotool.h"

#define FIFO_SOUND_READY 0x1234

extern "C" {
	static volatile bool _fifoReady = false;
	static volatile int _batteryStatus = 0;

	void statusCallback(u32 msg, void *userdata) {
		u32 key = msg >> 16;
		u32 value = msg & 0xffff;

		if (key == FIFO_SOUND_READY) {
			if (!_fifoReady) {
				soundEnable();
			}

			_fifoReady = true;
		}

		if (key == MENU_MSG_BATTERY_STATE) {
			_batteryStatus = _batteryStatus | value;
		}
	}

	void registerFifoHandlers() {
		fifoSetValue32Handler(FIFO_USER_03, statusCallback, NULL);
	}
}

cSystemDetails::cSystemDetails() {
    _fifoInit = false;
	_chargingStatus = false;
}

void cSystemDetails::initArm7RegStatuses() {
	if (!_fifoInit) {
		registerFifoHandlers();
	}
    
	_fifoInit = true;
}

void cSystemDetails::update() {
	u32 level = getBatteryLevel();
	_chargingStatus = (level & 0x80) != 0;
	
	// FILE* f = fopen("battery.txt", "a");
	// if (f != NULL) {
	// 	fprintf(f, "Battery=%d, charging=%ld\n", _batteryStatus, level);
	// }

	// fclose(f);

	if (_chargingStatus) {
		_batteryStatus = 0;
	}
}

bool cSystemDetails::fifoStatus() {
	return _fifoReady;
}

int cSystemDetails::batteryStatus() {
	return (_batteryStatus & 0x1) | (_chargingStatus ? 2 : 0);
}