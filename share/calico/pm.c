#include <unistd.h>
#include <nds/fifocommon.h>
#if defined(ARM9)
#include <nds/arm9/cp15.h>
#endif
#include "pm.h"
#include "env.h"
#include "pxi.h"
#include "common.h"
#include "irq.h"
#include "../fifotool.h"

static volatile bool running = true;

bool pmMainLoop(void) {
    return running;
}

void flushConsole(void) {
    fprintf(stdout, "\n");
    fflush(stderr);
    fflush(stdout);
}

void systemErrorExit(int rc)
{
    fprintf(stderr, "Exit called with code %d.\n", rc);

    if (rc != 303) {
        flushConsole();

        return;
    }

#if defined(ARM9)
    fifoSendValue32(FIFO_USER_01, MENU_MSG_ARM7_STOP);
    fifoWaitValue32(FIFO_USER_07);
    bool arm7state = fifoGetValue32(FIFO_USER_07);

    if (arm7state) {
        fprintf(stderr, "Loop of arm7 did not complete.\n");

        flushConsole();

        return;
    }

    fprintf(stderr, "Start jump preparations in arm9.");
    flushConsole();

    armIrqLockByPsr();
    irqLock();

    CP15_CleanAndFlushDCache();
    
    // Retrieve jump target address
	void (* jump_target)(void);
	if (g_envExtraInfo->pm_chainload_flag == 1) {
		jump_target = (void(*)(void)) g_envAppNdsHeader->arm9_entrypoint;

        fprintf(stderr, "Start PXI sync in arm9.");
        flushConsole();

		// // Perform PXI sync sequence
		// while (PXI_SYNC_RECV(REG_PXI_SYNC) != 1);
		// REG_PXI_SYNC = PXI_SYNC_SEND(1);
		// while (PXI_SYNC_RECV(REG_PXI_SYNC) != 0);
		// REG_PXI_SYNC = PXI_SYNC_SEND(0);

        // fprintf(stderr, "Complete PXI sync in arm9.");
        // flushConsole();
	} else {
		jump_target = g_envNdsBootstub->arm9_entrypoint;
	}

    fprintf(stderr, "Disabling caches in arm9.");
    flushConsole();

    disableCachesAndPU();

    fprintf(stderr, "Jumping in arm9.");
    flushConsole();

    jump_target();
	for (;;); // just in case
#elif defined(ARM7)
    fprintf(stderr, "Start jump preparations in arm7.");
    flushConsole();

    armIrqLockByPsr();
    irqLock();

    // // Perform PXI sync sequence
	// REG_PXI_SYNC = PXI_SYNC_SEND(1);
	// while (PXI_SYNC_RECV(REG_PXI_SYNC) != 1);
	// REG_PXI_SYNC = PXI_SYNC_SEND(0);
	// while (PXI_SYNC_RECV(REG_PXI_SYNC) != 0);

    fprintf(stderr, "Start clearing PXI fifo in arm9.");
    flushConsole();

	// Clear PXI FIFO
	while (!(REG_PXI_CNT & PXI_CNT_RECV_EMPTY)) {
		MK_DUMMY(REG_PXI_RECV);
	}

    fprintf(stderr, "Jumping in arm7.");
    flushConsole();

	// Jump to new ARM7 entrypoint
	if (g_envExtraInfo->pm_chainload_flag == 1) {
		pmJumpToNextApp();
	}
#endif

    flushConsole();
}