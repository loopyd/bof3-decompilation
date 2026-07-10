#ifndef BOF3_COMPILER_H
#define BOF3_COMPILER_H

/* Prevent sibling calls so the compiler emits exact jal instructions
 * matching the original binary's call graph. */
#if defined(__GNUC__)
#define NO_SIBLING_CALLS __attribute__((optimize("no-optimize-sibling-calls")))
#else
#define NO_SIBLING_CALLS
#endif

#endif
