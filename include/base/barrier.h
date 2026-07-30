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
 * These constrain caller-register scheduling for a C-generated instruction,
 * including jal/branch delay-slot placement and fixed-address reload ordering.
 * They do not emit or select an opcode; use them only after asm-diff identifies
 * the required register and placement.
 */
#if defined(__GNUC__)
#define CLOBBER_CALLER_REG(reg) CLOBBER_CALLER_REG_##reg()
#define CLOBBER_CALLER_REG_a0() __asm__ __volatile__("" : : : "a0")
#define CLOBBER_CALLER_REG_a1() __asm__ __volatile__("" : : : "a1")
#define CLOBBER_CALLER_REG_a2() __asm__ __volatile__("" : : : "a2")
#define CLOBBER_CALLER_REG_a3() __asm__ __volatile__("" : : : "a3")
#define CLOBBER_CALLER_REG_v0() __asm__ __volatile__("" : : : "v0")
#define CLOBBER_CALLER_REG_v1() __asm__ __volatile__("" : : : "v1")
#define CLOBBER_CALLER_REG_t0() __asm__ __volatile__("" : : : "t0")
#define CLOBBER_CALLER_REG_t1() __asm__ __volatile__("" : : : "t1")
#define CLOBBER_CALLER_REG_t2() __asm__ __volatile__("" : : : "t2")
#define CLOBBER_CALLER_REG_t3() __asm__ __volatile__("" : : : "t3")
#define CLOBBER_CALLER_REG_t4() __asm__ __volatile__("" : : : "t4")
#define CLOBBER_CALLER_REG_t5() __asm__ __volatile__("" : : : "t5")
#define CLOBBER_CALLER_REG_t6() __asm__ __volatile__("" : : : "t6")
#define CLOBBER_CALLER_REG_t7() __asm__ __volatile__("" : : : "t7")
#define CLOBBER_CALLER_REG_t8() __asm__ __volatile__("" : : : "t8")
#define CLOBBER_CALLER_REG_t9() __asm__ __volatile__("" : : : "t9")
/* Keep established wrappers spelled directly: cc1 scheduling is spelling-sensitive. */
#define CLOBBER_A0() __asm__ __volatile__("" : : : "a0")
#define CLOBBER_V0() __asm__ __volatile__("" : : : "v0")
#define CLOBBER_V1() __asm__ __volatile__("" : : : "v1")
#define CLOBBER_A1() __asm__ __volatile__("" : : : "a1")
#define CLOBBER_A2() __asm__ __volatile__("" : : : "a2")
#else
#define CLOBBER_CALLER_REG(reg)
#define CLOBBER_A0()
#define CLOBBER_V0()
#define CLOBBER_V1()
#define CLOBBER_A1()
#define CLOBBER_A2()
#endif

#endif
