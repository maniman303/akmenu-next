/*
    userinput.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "userinput.h"
#include <cstring>
#include "dbgtool.h"
#include "keymessage.h"
#include "touchmessage.h"
#include "timer.h"
#include "windowmanager.h"

using namespace akui;

static INPUT inputs;
static INPUT lastInputs;

void initInput() {
    keysSetRepeat(30, 1);
}

INPUT& updateInput() {
    memset(&inputs, 0, sizeof(inputs));
    touchRead(&inputs.touchPt);
    if (inputs.touchPt.px == 0 && inputs.touchPt.py == 0) {
        if (lastInputs.touchHeld) {
            inputs.touchUp = true;
            inputs.touchPt = lastInputs.touchPt;
            dbg_printf("getInput() Touch UP! %d %d\n", inputs.touchPt.px, inputs.touchPt.py);
        } else {
            inputs.touchUp = false;
        }
        inputs.touchDown = false;
        inputs.touchHeld = false;
    } else {
        if (!lastInputs.touchHeld) {
            inputs.touchDown = true;
            dbg_printf("getInput() Touch DOWN! %d %d\n", inputs.touchPt.px, inputs.touchPt.py);
        } else {
            inputs.movedPt.px = inputs.touchPt.px - lastInputs.touchPt.px;
            inputs.movedPt.py = inputs.touchPt.py - lastInputs.touchPt.py;
            inputs.touchMoved = (0 != inputs.movedPt.px) || (0 != inputs.movedPt.py);
            inputs.touchDown = false;
        }
        inputs.touchUp = false;
        inputs.touchHeld = true;
    }
    // dbg_printf( "touch x %d y %d\n", inputs.touchPt.px, inputs.touchPt.py );
    // dbg_printf( "touchdown %d clicked %d\n", inputs.touchDown, inputs.clicked );
    scanKeys();
    inputs.keysDown = keysDown();
    inputs.keysUp = keysUp();
    inputs.keysHeld = keysHeld();
    inputs.keysDownRepeat = keysDownRepeat();
    lastInputs = inputs;

    return inputs;
}

INPUT& getInput() {
    return inputs;
}

bool processInput(INPUT& inputs) {
    bool ret = false;

    ret = ret || windowManager().processKeyMessage(cKeyMessage(inputs.keysHeld, inputs.keysUp, inputs.keysDown, inputs.keysDownRepeat));
    ret = ret || windowManager().processTouchMessage(cTouchMessage(inputs.touchPt.px, inputs.touchPt.py, inputs.movedPt.px, inputs.movedPt.py, inputs.touchDown, inputs.touchUp, inputs.touchMoved));

    if (inputs.keysDown & KEY_LID) {
        dbg_printf("lid closed\n");
        fifoSendValue32(FIFO_PM, PM_REQ_SLEEP);
        swiDelay(8380000);  // 500ms
        /*
        powerOff(0x3f);
        powerOn(0x10);
        */
    } else if (inputs.keysUp & KEY_LID) {
        dbg_printf("lid opened\n");
        /*
        powerOff(0x3f);
        powerOn(0x0f);
        */
    }

    return ret;
}
