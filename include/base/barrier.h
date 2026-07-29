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

/*
 * Last-resort allocator constraint. Use only with function-specific user
 * approval and a MATCHING_AID comment; reg is a GCC MIPS register name.
 */
#define REGISTER_PIN(type, name, reg) register type name asm(reg)
/*
 * The macro form is supported by the current PsyQ compiler for ordinary
 * allocator pins. Legacy pins using numeric "$N" register spellings remain
 * direct declarations until their compiler syntax is separately verified.
 */
#else
#define NO_SIBLING_CALLS
#define barrier()
#define REGISTER_PIN(type, name, reg) register type name
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
#define CLOBBER_V1() __asm__ __volatile__("" : : : "v1")
#define CLOBBER_A1() __asm__ __volatile__("" : : : "a1")
#define CLOBBER_A2() __asm__ __volatile__("" : : : "a2")
#else
#define CLOBBER_A0()
#define CLOBBER_V0()
#define CLOBBER_V1()
#define CLOBBER_A1()
#define CLOBBER_A2()
#endif

#endif
