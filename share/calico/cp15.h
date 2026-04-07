#pragma once

//! @brief Cache line size on the DS/3DS's Arm9 CPU's data cache
#define ARM_CACHE_LINE_SZ 32

#define CP15_CR_PU_ENABLE     (1<<0)    //!< Enable the protection unit
#define CP15_CR_DCACHE_ENABLE (1<<2)    //!< Enable data cache
#define CP15_CR_ICACHE_ENABLE (1<<12)   //!< Enable instruction cache
#define CP15_CR_ROUND_ROBIN   (1<<14)   //!< If set, use a round-robin cache replacement algo, otherwise it is pseudo-random

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((target("arm"))) void disableCachesAndPU(void);

#ifdef __cplusplus
}
#endif