#pragma once
#include <stdint.h>
#include "mm.h"

//! @brief Marks a function as never being eligible for automatic inlining by the compiler
#define MK_NOINLINE   __attribute__((noinline))

//! @brief Marks a function as never returning to the caller
#define MK_NORETURN   __attribute__((noreturn))

//! @brief Marks a small, utility function whose contents will be inserted into the caller ("inlined").
#define MK_INLINE     __attribute__((always_inline)) static inline

/*! @brief Similar to @ref MK_INLINE, but allowing the function body to be
	emitted in an object file when explicitly requested.
	@note This is mostly useful for ASM wrappers that can be inline in ARM mode
	but not in THUMB mode. In addition, this feature is only supported by C (not C++).
*/
#define MK_EXTINLINE  __attribute__((always_inline)) inline

/*! @brief Accesses the specified memory-mapped register of the specified type.
	@param[in] _type Name of the type
	@param[in] _off Offset of the register (relative to the I/O register base)
*/
#define MK_REG(_type,_off) (*(_type volatile*)(MM_IO + (_off)))

//! @brief Macro used to explicitly express that a given variable is unused
#define MK_DUMMY(_x)  (void)(_x)

/*! @brief Marks an external function as being 32-bit ARM code
	@note This is an optional optimization only meaningful for ARMv4 (ARM7)
	code compiled in THUMB mode. The GNU linker will automatically generate
	appropriate stubs for interworking when this macro is not used.
*/
#if __thumb__ && __ARM_ARCH < 5 && !__clang__
#define MK_EXTERN32   __attribute__((long_call))
#else
#define MK_EXTERN32
#endif

typedef uintptr_t uptr;    //!< Pointer-sized unsigned integer.