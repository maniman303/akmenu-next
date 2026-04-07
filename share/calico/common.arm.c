#include "common.h"

u32 armGetCpsr(void)
{
	u32 reg;
	__asm__ __volatile__ ("mrs %0, cpsr" : "=r" (reg));
	return reg;
}

void armSetCpsrC(u32 value)
{
	__asm__ __volatile__ ("msr cpsr_c, %0" :: "r" (value) : "memory");
}