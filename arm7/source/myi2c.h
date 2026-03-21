/*---------------------------------------------------------------------------------

	I2C control for the ARM7

	Copyright (C) 2011
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/

#pragma once

#include <nds/arm7/i2c.h>
#include <nds/bios.h>

//---------------------------------------------------------------------------------
void my_i2cDelay();
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
void my_i2cStop(u8 arg0);
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
u8 my_i2cGetResult();
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
u8 my_i2cGetData();
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
void my_i2cSetDelay(u8 device);
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
u8 my_i2cSelectDevice(u8 device);
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
u8 my_i2cSelectRegister(u8 reg);
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
u8 my_i2cWriteRegister(u8 device, u8 reg, u8 data);
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
u8 my_i2cReadRegister(u8 device, u8 reg);
//---------------------------------------------------------------------------------