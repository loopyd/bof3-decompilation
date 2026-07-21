#ifndef MEMORY_REGISTERS_H
#define MEMORY_REGISTERS_H

#include "memory/access.h"

/*
 * PS1 memory-mapped hardware registers.
 *
 * These are for hardware registers, not scratchpad RAM.
 * Use scratchpad.h for 0x1F800000-0x1F8003FF accesses.
 */
#define REG8(address) PSX_REF(volatile u8, address)

#define REG16(address) PSX_REF(volatile u16, address)

#define REG32(address) PSX_REF(volatile u32, address)

#endif
