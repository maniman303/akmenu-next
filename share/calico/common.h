#pragma once

#include "types.h"

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