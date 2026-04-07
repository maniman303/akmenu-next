#pragma once

//! Homebrew bootstub area, used to implement return-to-hbmenu
#define MM_ENV_HB_BOOTSTUB        0x2ff4000
#define MM_ENV_HB_BOOTSTUB_SZ     0x8000 //!< Size of @ref MM_ENV_HB_BOOTSTUB (32kb)

//! NDS ROM header for the currently running application
#define MM_ENV_APP_NDS_HEADER     0x2fffe00

//! DSi ROM header for the currently running application
#define MM_ENV_APP_TWL_HEADER     0x2ffe000

#define MM_TWLWRAM_MAP       0x3000000
#define MM_TWLWRAM_BANK_SZ     0x40000 // 256kb
#define MM_TWLWRAM_SLOT_A_SZ   0x10000 // 64kb
#define MM_TWLWRAM_SLOT_B_SZ    0x8000 // 32kb
#define MM_TWLWRAM_SLOT_C_SZ    0x8000 // 32kb

//! Free area reserved for internal use
#define MM_ENV_FREE_FDA0          0x2fffda0
#define MM_ENV_FREE_FDA0_SZ       0x50 //!< Size of @ref MM_ENV_FREE_FDA0