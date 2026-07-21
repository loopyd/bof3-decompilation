#ifndef BASE_BARRIER_H
#define BASE_BARRIER_H

#include "base/types.h"

/*
 * Prevent sibling-call optimization when the original binary contains
 * a real call instruction rather than a tail call.
 */
#if defined(__GNUC__)
#define NO_SIBLING_CALLS __attribute__((optimize("no-optimize-sibling-calls")))

#define barrier() __asm__ __volatile__("" : : : "memory")
#else
#define NO_SIBLING_CALLS
#define barrier()
#endif

/*
 * Register-clobber barriers for MIPS delay-slot ordering.
 *
 * These prevent the compiler from hoisting a register assignment out
 * of a jal/branch delay slot, matching original binary codegen.
 */
#if defined(__GNUC__)
#define CLOBBER_A0() __asm__ __volatile__("" : : : "a0")
#define CLOBBER_V0() __asm__ __volatile__("" : : : "v0")
#define CLOBBER_A1() __asm__ __volatile__("" : : : "a1")
#define CLOBBER_A2() __asm__ __volatile__("" : : : "a2")
#else
#define CLOBBER_A0()
#define CLOBBER_V0()
#define CLOBBER_A1()
#define CLOBBER_A2()
#endif

#endif
