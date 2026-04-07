#pragma once

#include <nds.h>
#include "mm.h"
#include "mm_env.h"
#include "cp15.h"
#include "dldi_defs.h"
#include "scfg.h"

#define g_envAppNdsHeader  ((EnvNdsHeader*)         MM_ENV_APP_NDS_HEADER)
#define g_envAppTwlHeader  ((EnvTwlHeader*)         MM_ENV_APP_TWL_HEADER)
#define g_envNdsBootstub   ((EnvNdsBootstubHeader*) MM_ENV_HB_BOOTSTUB)
#define g_envExtraInfo     ((EnvExtraInfo*)         MM_ENV_FREE_FDA0)

//! Extra information in shared main RAM maintained by calico
typedef struct EnvExtraInfo {
	u16 nvram_offset_div8;
	u8  nvram_console_type; //!< @ref EnvConsoleType
	u8  wlmgr_rssi;
	u8  wlmgr_macaddr[6];
	u16 wlmgr_channel_mask;
	u8  pm_chainload_flag;
	u8  wlmgr_hdr_headroom_sz;
	u16 dldi_features;
	u32 dldi_io_type;
} EnvExtraInfo;

//! NDS ROM header debug fields
typedef struct EnvNdsHeaderDebugFields {
	u32 debug_rom_offset;
	u32 debug_size;
	u32 debug_ram_address;
	u32 _pad_0x0C;
} EnvNdsHeaderDebugFields;

//! NDS ROM header structure
typedef struct EnvNdsHeader {
	char title[12];
	char gamecode[4];
	char makercode[2];
	u8 unitcode;
	u8 encryption_seed_select;
	u8 device_capacity;
	u8 _pad_0x15[7];
	u8 twl_flags;
	u8 ntr_region_flags;
	u8 rom_version;
	u8 ntr_flags;

	u32 arm9_rom_offset;
	u32 arm9_entrypoint;
	u32 arm9_ram_address;
	u32 arm9_size;

	u32 arm7_rom_offset;
	u32 arm7_entrypoint;
	u32 arm7_ram_address;
	u32 arm7_size;

	u32 fnt_rom_offset;
	u32 fnt_size;
	u32 fat_rom_offset;
	u32 fat_size;
	u32 arm9_ovl_rom_offset;
	u32 arm9_ovl_size;
	u32 arm7_ovl_rom_offset;
	u32 arm7_ovl_size;

	u32 cardcnt_normal;
	u32 cardcnt_key1;

	u32 banner_rom_offset;

	u16 secure_area_crc16;
	u16 secure_area_delay;

	u32 arm9_loadlist_hook;
	u32 arm7_loadlist_hook;

	u32 secure_area_disable_magic[2];

	u32 ntr_rom_size;
	u32 rom_header_size;

	u32 arm9_param_rom_offset;
	u32 arm7_param_rom_offset;

	u32 _uninteresting_0x90[0x30/4];

	u8 nintendo_logo[0x9C];
	u16 nintendo_logo_crc16;
	u16 header_crc16;
} EnvNdsHeader;

//! DSi ROM header structure
typedef struct EnvTwlHeader {
	EnvNdsHeader base;
	EnvNdsHeaderDebugFields debug;

	u8 _pad_0x170[0x10];

	u32 mbk_slot_settings[5];
	u32 arm9_mbk_map_settings[3];
	u32 arm7_mbk_map_settings[3];
	struct {
		u32 mbk_slotwrprot_setting : 24;
		u32 wramcnt_setting : 8;
	};

	u32 twl_region_flags;
	u32 twl_access_control;
	u32 scfg_ext7_setting;

	u8 _pad_0x1BC[3];
	u8 twl_flags2;

	u32 arm9i_rom_offset;
	u32 _unused_0x1C4;
	u32 arm9i_ram_address;
	u32 arm9i_size;

	u32 arm7i_rom_offset;
	u32 device_list_ram_address;
	u32 arm7i_ram_address;
	u32 arm7i_size;

	u32 ntr_digest_rom_start;
	u32 ntr_digest_size;
	u32 twl_digest_rom_start;
	u32 twl_digest_size;
	u32 digest_level1_rom_offset;
	u32 digest_level1_size;
	u32 digest_level0_rom_offset;
	u32 digest_level0_size;
	u32 digest_sector_size;
	u32 digest_sectors_per_block;

	u32 twl_banner_size;
	u32 _uninteresting_0x20C;
	u32 twl_rom_size;
	u32 _uninteresting_0x214;

	u32 arm9i_param_rom_offset;
	u32 arm7i_param_rom_offset;

	u32 arm9_modcrypt_rom_start;
	u32 arm9_modcrypt_size;
	u32 arm7_modcrypt_rom_start;
	u32 arm7_modcrypt_size;

	union {
		u64 title_id;
		struct {
			u32 title_id_low;
			u32 title_id_high;
		};
	};
	u32 public_sav_size;
	u32 private_sav_size;

	u8 _pad_0x240[0xb0];

	u8 age_ratings[0x10];

	u8 hmac_arm9[20];
	u8 hmac_arm7[20];
	u8 hmac_digest_level0[20];
	u8 hmac_banner[20];
	u8 hmac_arm9i[20];
	u8 hmac_arm7i[20];

	u8 hmac_old_ntr_1[20];
	u8 hmac_old_ntr_2[20];
	u8 hmac_arm9_no_secure[20];

	u8 _pad_0x3B4[0xa4c];

	//-------------------------------------------------------------------------

	u8 twl_debug_args[0x180];
	u8 rsa_sha1_signature[0x80];
} EnvTwlHeader;

//! Homebrew bootstub header (used for ret2hbmenu)
typedef struct EnvNdsBootstubHeader {
	u64 magic; //!< @ref ENV_NDS_BOOTSTUB_MAGIC

	//! Main return-to-hbmenu entrypoint, for use on ARM9.
	MK_NORETURN void (*arm9_entrypoint)(void);

	//! This entrypoint is intended for requesting return-to-hbmenu directly from ARM7.
	void (*arm7_entrypoint)(void);
} EnvNdsBootstubHeader;