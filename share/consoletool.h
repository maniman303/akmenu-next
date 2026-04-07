#pragma once

#include <stdio.h>

inline void flushConsole() {
    fprintf(stdout, "\n");
    fflush(stderr);
    fflush(stdout);
}