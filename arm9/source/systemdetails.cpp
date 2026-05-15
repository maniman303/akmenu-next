#include <nds/arm9/dldi.h>
#include <nds.h>
#include <cstdio>
#include "systemdetails.h"
#include "fifotool.h"
#include "logger.h"

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
	bool oldCharging = _chargingStatus;
	int oldStatus = _batteryStatus;

	u32 level = getBatteryLevel();
	_chargingStatus = (level & 0x80) != 0;

	if (_chargingStatus) {
		_batteryStatus = 15;
	} else {
		_batteryStatus = level & 0xf;
	}

	if (oldCharging == _chargingStatus && oldStatus == _batteryStatus) {
		return;
	}

	if (_batteryStatus > 3 || _chargingStatus) {
		ledBlink(PM_LED_ON);
	} else {
		ledBlink(PM_LED_BLINK);
	}
}

bool cSystemDetails::fifoStatus() {
	return _fifoReady;
}

bool cSystemDetails::isBatteryLow() {
	return _batteryStatus <= 3;
}

bool cSystemDetails::isBatteryCharging() {
	return _chargingStatus;
}