// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the BlocksDS project.
//
// Copyright (C) 2005 Jason Rogers (dovoto)
// Copyright (C) 2005 Dave Murphy (WinterMute)
// Copyright (C) 2023 Antonio Niño Díaz

/// @file nds/dma.h
///
/// @brief Wrapper functions for direct memory access hardware.
///
/// The DS has 4 hardware direct memory access devices per CPU which can be used
/// to transfer or fill chunks of memeory without CPU intervention. Using DMA is
/// generaly faster than CPU copies (memcpy, swiCopy, for loops, etc..).
///
/// DMA has no access to data caches on the DS and as such will give unexpected
/// results when DMAing data from main memory. The cache must be flushed as
/// follows when using DMA to ensure proper opertion on the ARM9:
///
/// ```
/// DC_FlushRange(source, sizeof(dataToCopy));
/// dmaCopy(source, destination, sizeof(dataToCopy));
/// ```
///
/// Very important note: Don't write to the DMA registers directly from the
/// ARM9, it is unsafe. Enabling and disabling the DMA must not be done from
/// code in main RAM, and special care is required to disable an enabled DMA
/// channel. You should use dmaSetParams() and dmaStopSafe() instead.

#ifndef LIBNDS_NDS_DMA_H__
#define LIBNDS_NDS_DMA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <nds/ndstypes.h>
/// Stops a DMA channel safely from ITCM.
///
/// @param channel
///     The DMA channel to use (0 - 3).
void dmaStopSafe(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif // LIBNDS_NDS_DMA_H__