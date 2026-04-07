#pragma once

#include <nds/arm9/dldi_asm.h>

#define DLDI_SIZE_MAX          DLDI_SIZE_16KB
// #define DLDI_SIZE_16KB         14
// #define DLDI_SIZE_8KB          13
// #define DLDI_SIZE_4KB          12
// #define DLDI_SIZE_2KB          11
// #define DLDI_SIZE_1KB          10

#define DLDI_MAX_ALLOC_SZ      (1U<<DLDI_SIZE_MAX)