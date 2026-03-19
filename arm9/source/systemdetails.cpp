#include <nds/arm9/dldi.h>
#include <nds.h>
#include "systemdetails.h"

extern "C" {
	static volatile bool _fifoReady = false;

	void soundCallback(u32 value, void *userdata) {
		if (value == 0x1234) {
			soundEnable();

			_fifoReady = true;
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