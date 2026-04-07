#include <nds.h>
#include "pm.h"
#include "env.h"
#include "mm.h"
#include "common.h"

void pmJumpToNextApp(void) {
#if defined(ARM7)
    // Back up DSi mode flag
	bool isDsi = isDSiMode();
	armCompilerBarrier();

	// Copy new ARM7 binary to WRAM if needed
	if (g_envAppNdsHeader->arm7_ram_address >= MM_A7WRAM && g_envAppNdsHeader->arm7_ram_address < MM_IO) {
		vu32* dst = (vu32*)g_envAppNdsHeader->arm7_ram_address; // volatile to avoid memcpy optimization
		u32* src = (u32*)(MM_MAINRAM + MM_MAINRAM_SZ_NTR - 512*1024);
		u32 count = (g_envAppNdsHeader->arm7_size + 3) / 4;
		do {
			*dst++ = *src++;
		} while (count--);
	}

	// Set up MBK regs if needed
	if (isDsi) {
		REG_MBK_SLOTWRPROT = g_envAppTwlHeader->mbk_slotwrprot_setting;
		REG_MBK_MAP_A = g_envAppTwlHeader->arm7_mbk_map_settings[0];
		REG_MBK_MAP_B = g_envAppTwlHeader->arm7_mbk_map_settings[1];
		REG_MBK_MAP_C = g_envAppTwlHeader->arm7_mbk_map_settings[2];
	}

	// Jump to ARM7 entrypoint
	((void(*)(void))g_envAppNdsHeader->arm7_entrypoint)();
#endif
    for (;;); // just in case
}