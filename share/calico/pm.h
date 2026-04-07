#pragma once

#include <stdbool.h>
#include <stdio.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void flushConsole(void);

MK_EXTERN32 MK_NOINLINE MK_NORETURN void pmJumpToNextApp(void);

bool pmMainLoop(void);

#ifdef __cplusplus
}
#endif