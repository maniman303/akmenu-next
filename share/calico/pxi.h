#pragma once

#include "io.h"
#include "types.h"

#define REG_PXI_SYNC MK_REG(u16, IO_PXI_SYNC)
#define REG_PXI_CNT  MK_REG(u32, IO_PXI_CNT)
#define REG_PXI_SEND MK_REG(u32, IO_PXI_SEND)
#define REG_PXI_RECV MK_REG(u32, IO_PXI_RECV)

#define PXI_CNT_RECV_EMPTY      (1U << 8)

#define PXI_SYNC_RECV(_n)   ((_n) & 0xF)
#define PXI_SYNC_SEND(_n)   (((_n) & 0xF) << 8)