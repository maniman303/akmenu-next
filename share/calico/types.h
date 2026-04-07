#pragma once
#include <cstdint>
#include "mm.h"

//! @brief Marks a function as never returning to the caller
#define MK_NORETURN   __attribute__((noreturn))

//! @brief Marks a small, utility function whose contents will be inserted into the caller ("inlined").
#define MK_INLINE     __attribute__((always_inline)) static inline

/*! @brief Accesses the specified memory-mapped register of the specified type.
	@param[in] _type Name of the type
	@param[in] _off Offset of the register (relative to the I/O register base)
*/
#define MK_REG(_type,_off) (*(_type volatile*)(MM_IO + (_off)))

typedef uintptr_t uptr;    //!< Pointer-sized unsigned integer.