#ifndef BOF3_ASM_H
#define BOF3_ASM_H

/*
 * Unmatched function fallback.
 *
 * Provides a first-class path for functions that cannot yet be represented in
 * clean matching C. Preserves:
 *     - section selection and alignment
 *     - .set noreorder / .set noat directives
 *     - symbol metadata (.globl, .ent/.end)
 *     - separate read-only data inclusion
 *
 * This enables incremental reconstruction without producing low-quality fake
 * matches. A clean unmatched function is better than unreadable C filled with
 * arbitrary hacks.
 *
 * ---- Usage ----
 *
 * When a function cannot be matched, replace the C body with INCLUDE_ASM:
 *
 *     #include "internal.h"
 *     #include "bof3/asm.h"
 *
 *     // func_8014AEE0 is not yet matched — see adjacent .s file.
 *     INCLUDE_ASM(func_8014AEE0);
 *
 * The macro marks the call site with a global symbol declaration. The actual
 * implementation lives in an adjacent assembly file compiled into the same
 * target:
 *
 *     src/exe/<target>/func_XXXXXXXX.c    (declarations + INCLUDE_ASM marker)
 *     src/exe/<target>/func_XXXXXXXX.s    (raw disassembly)
 *
 * When reconstruction succeeds, replace both files with matching C and remove
 * the .s file. The Splat boundary transitions from "a" (asm) to "c" (C).
 *
 * ---- Assembly file format ----
 *
 *     .set noreorder
 *     .set noat
 *
 *     .section .text.func_XXXXXXXX, "ax", @progbits
 *     .align 2
 *     .globl func_XXXXXXXX
 *     .ent   func_XXXXXXXX
 *     func_XXXXXXXX:
 *         # raw disassembly here — preserve original instruction order
 *     .end   func_XXXXXXXX
 *     .set reorder
 *
 * The section name (.text.func_XXXXXXXX) must match the Splat boundary to
 * ensure correct placement in the linked binary. Use "ax" flags for code,
 * "aw" @nobits for BSS, and "a" @progbits for read-only data.
 *
 * For functions with adjacent .rodata (jump tables, string literals):
 *     - Include the .rodata section before .text in the same .s file, or
 *     - Place it in a companion func_XXXXXXXX.rodata.s file and include both
 *       from the build system.
 *
 * ---- Build integration ----
 *
 * The build system must compile adjacent .s files alongside their .c
 * counterparts for each target directory containing INCLUDE_ASM markers.
 * See Makefile or build scripts for per-target assembly inclusion rules.
 */

#ifndef INCLUDE_ASM
#define INCLUDE_ASM(name) __asm__( ".globl " #name )
#endif

#endif
