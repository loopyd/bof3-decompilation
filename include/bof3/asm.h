#ifndef BOF3_ASM_H
#define BOF3_ASM_H

/*
 * Unmatched-function fallback.
 *
 * Provides a first-class path for functions that cannot yet be represented in
 * clean matching C. The macros pull the original disassembly from an adjacent
 * `.s` file compiled into the same translation unit, preserving:
 *     - section selection and alignment
 *     - .set noreorder / .set noat directives
 *     - symbol metadata (.globl, .ent/.end)
 *     - separate read-only data inclusion
 *
 * This enables incremental reconstruction without producing low-quality fake
 * matches. A clean unmatched function is better than unreadable C held
 * together by undocumented register hacks.
 *
 * ---- Usage ----
 *
 * When a function cannot be matched, replace its C body with INCLUDE_ASM and
 * keep the original disassembly in an adjacent .s file:
 *
 *     // func_8014AEE0 is not yet matched — see adjacent .s file.
 *     INCLUDE_ASM("asm/nonmatchings/battle", func_8014AEE0);
 *
 *     src/exe/<target>/func_XXXXXXXX.c    (declarations + INCLUDE_ASM marker)
 *     asm/nonmatchings/<module>/func_XXXXXXXX.s  (raw disassembly)
 *
 * When reconstruction succeeds, replace the .c with matching C and remove the
 * .s file. The Splat boundary transitions from "a" (asm) to "c" (C).
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
 * For functions with adjacent .rodata (jump tables, string literals):
 *
 *     INCLUDE_RODATA("asm/nonmatchings/battle", func_XXXXXXXX);
 *
 * ---- Tooling modes ----
 *
 * Under M2CTX or PERMUTER (context extraction / permuter search) the macros
 * expand to nothing so the C body is analyzed in isolation without pulling in
 * raw assembly.
 */

#if !defined(M2CTX) && !defined(PERMUTER)

#ifndef INCLUDE_ASM
#define INCLUDE_ASM(FOLDER, NAME)        \
  __asm__(                               \
      ".section .text\n"                 \
      "    .set noat\n"                  \
      "    .set noreorder\n"             \
      "    .include \"" FOLDER "/" #NAME \
      ".s\"\n"                           \
      "    .set reorder\n"               \
      "    .set at\n")
#endif

#ifndef INCLUDE_RODATA
#define INCLUDE_RODATA(FOLDER, NAME)     \
  __asm__(                               \
      ".section .rodata\n"               \
      "    .include \"" FOLDER "/" #NAME \
      ".s\"\n"                           \
      ".section .text")
#endif

#else /* M2CTX or PERMUTER */

#ifndef INCLUDE_ASM
#define INCLUDE_ASM(FOLDER, NAME)
#endif

#ifndef INCLUDE_RODATA
#define INCLUDE_RODATA(FOLDER, NAME)
#endif

#endif /* !defined(M2CTX) && !defined(PERMUTER) */

/*
 * Mark a function body as intentionally not matched.
 * Use to annotate a C stub that is a placeholder, distinct from a clean
 * partial match. Expands to nothing in normal builds.
 */
#ifndef NON_MATCHING
#define NON_MATCHING
#endif

/*
 * Skip compiling the following function from C; its implementation lives
 * entirely in assembly. Pair with INCLUDE_ASM for the active definition.
 */
#ifndef SKIP_ASM
#define SKIP_ASM
#endif

#endif /* BOF3_ASM_H */
