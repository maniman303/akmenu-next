#pragma once

#include <nds.h>
#include "types.h"
#include "psr.h"

/*! @brief Prevents the compiler from reordering memory accesses around the call to this function
	@note This barrier is intended to be used when the compiler has no way to infer that the code
	may be interrupted by other events and thus cause memory locations to be updated, or when other
	processors/devices are intended to observe said updates. Compiler barriers also implicitly occur
	when calling an external (non-inline) function.
*/
MK_INLINE void armCompilerBarrier(void)
{
	__asm__ __volatile__ ("" ::: "memory");
}

//! Saved state of the CPSR IRQ/FIQ mask bits @see ARM_PSR_I, ARM_PSR_F
typedef unsigned ArmIrqState;

//! @brief Retrieves the value of the Current Program Status Register
u32 armGetCpsr(void);

//! @brief Sets the control bits (execution mode, IRQ/FIQ mask) of the Current Program Status Register
void armSetCpsrC(u32 value);

/*! @brief Temporarily disables interrupts on the CPU
	@return Previous @ref ArmIrqState, to be later passed to @ref armIrqUnlockByPsr
*/
MK_EXTINLINE ArmIrqState armIrqLockByPsr(void)
{
	u32 psr = armGetCpsr();
	armSetCpsrC(psr | ARM_PSR_I | ARM_PSR_F);
	return psr & (ARM_PSR_I | ARM_PSR_F);
}