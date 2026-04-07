#pragma once

#include <nds/interrupts.h>
#include "common.h"

//! Saved state of `REG_IME`
typedef unsigned IrqState;

/*! @brief Temporarily disables interrupts on the interrupt controller (using `REG_IME`).
	@return Previous @ref IrqState, to be later passed to @ref irqUnlock
	@note @ref irqLock and @ref irqUnlock are best suited for simple atomic operations
	that involve no external function calls, especially in THUMB-mode code.
	@warning `REG_IME` is **not** part of the thread context, and is therefore not saved/restored
	during context switch. The threading subsystem **will malfunction** if any context switches
	occur while it is disabled. Consider using @ref armIrqLockByPsr and @ref armIrqUnlockByPsr
	instead if you intend to implement synchronization code, or call external functions.
*/
MK_INLINE IrqState irqLock(void)
{
	armCompilerBarrier();
	IrqState saved = REG_IME;
	REG_IME = 0;
	armCompilerBarrier();
	return saved;
}