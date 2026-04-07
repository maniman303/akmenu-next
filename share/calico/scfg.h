#pragma once

#include <nds.h>
#include "types.h"
#include "mm.h"
#include "io.h"

#define MBK_MAP_START(_n) (((_n)&0x1ff)<<3)
#define MBK_MAP_SIZE(_n)  (((_n)&3)<<12)
#define MBK_MAP_END(_n)   (((_n)&0x3ff)<<19)

#define MBK_MAP_GRANULARITY 0x8000

#define REG_MBK_MAP_A       MK_REG(u32, IO_MBK_MAP_A)       // arm7 rw, arm9 rw (per-cpu)
#define REG_MBK_MAP_B       MK_REG(u32, IO_MBK_MAP_B)
#define REG_MBK_MAP_C       MK_REG(u32, IO_MBK_MAP_C)
#define REG_MBK_SLOTWRPROT  MK_REG(u32, IO_MBK_SLOTWRPROT)  // arm7 rw, arm9 ro

//! New WRAM map size, for use with MBK_MAP_SIZE
typedef enum MbkMapSize {
	MbkMapSize_32K  = 0, // only for WRAM_B/C
	MbkMapSize_64K  = 1,
	MbkMapSize_128K = 2,
	MbkMapSize_256K = 3,
} MbkMapSize;

/*! @brief Calculates a configuration value for the REG_SCFG_MAP registers
	@param[in] start_addr Start of the address range to map
	@param[in] end_addr End of the address range to map (exclusive)
	@param[in] sz Image size of the bank (usually MbkMapSize_256K)
	@note The address range must be contained within the MM_TWLWRAM_MAP area.
	WRAM B/C addresses must be a multiple of `MBK_MAP_GRANULARITY`, while
	WRAM A addresses must be a multiple of `2*MBK_MAP_GRANULARITY`.
*/
MK_INLINE u32 mbkMakeMapping(uptr start_addr, uptr end_addr, MbkMapSize sz)
{
	unsigned start_off = (start_addr - MM_TWLWRAM_MAP) / MBK_MAP_GRANULARITY;
	unsigned end_off   = (end_addr   - MM_TWLWRAM_MAP) / MBK_MAP_GRANULARITY;
	return MBK_MAP_START(start_off) | MBK_MAP_SIZE(sz) | MBK_MAP_END(end_off);
}