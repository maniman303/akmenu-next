#pragma once

#define MM_IO          0x4000000 // 32-bit bus

#define MM_VRAM        0x6000000 // 16-bit bus

#define MM_VRAM_C      (MM_VRAM+0x840000)
#define MM_VRAM_C_SZ   0x20000 // 128kb

#define MM_TWLWRAM_MAP       0x3000000
#define MM_TWLWRAM_BANK_SZ     0x40000 // 256kb
#define MM_TWLWRAM_SLOT_A_SZ   0x10000 // 64kb
#define MM_TWLWRAM_SLOT_B_SZ    0x8000 // 32kb
#define MM_TWLWRAM_SLOT_C_SZ    0x8000 // 32kb

#define MM_MAINRAM        0x2000000 // 16-bit bus
#define MM_MAINRAM_SZ_NTR  0x400000 // 4mb
#define MM_MAINRAM_SZ_DBG  0x800000 // 8mb
#define MM_MAINRAM_SZ_TWL 0x1000000 // 16mb

#ifdef ARM7
#define MM_A7WRAM         0x37f8000 // 32-bit bus
#define MM_A7WRAM_SZ        0x18000 // 96kb
#endif