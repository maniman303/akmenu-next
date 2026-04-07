#include <nds.h>
#include "cp15.h"

__attribute__((target("arm"))) void disableCachesAndPU(void) {
    u32 cr;
    asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(cr));
    cr &= ~( (1u << 0)   // Protection unit
           | (1u << 2)   // D-cache
           | (1u << 12)  // Round-robin
           | (1u << 14)  // I-cache
           );
    asm volatile("mcr p15, 0, %0, c1, c0, 0" :: "r"(cr) : "memory");
}