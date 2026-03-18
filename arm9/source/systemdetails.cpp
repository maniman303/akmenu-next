#include <nds/arm9/dldi.h>
#include <nds.h>
#include "systemdetails.h"

#define BAT_MASK  0b00000000000011111111
#define BAT_OFF 0

#define VOL_MASK  0b00000001111100000000
#define VOL_OFF 8

extern "C" {
	static volatile int _batteryLevel = 0;
	static volatile int _volumeLevel = -1;

	void volBatSdCallback(u32 status, void *userdata) {
		_batteryLevel = (status & BAT_MASK) >> BAT_OFF;
		_volumeLevel = (status & VOL_MASK) >> VOL_OFF;
	}

	void registerFifoHandlers() {
		fifoSetValue32Handler(FIFO_USER_03, volBatSdCallback, NULL);
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

int cSystemDetails::batteryStatus() {
	return _batteryLevel;
}

int cSystemDetails::volumeStatus() {
	return _volumeLevel;
}