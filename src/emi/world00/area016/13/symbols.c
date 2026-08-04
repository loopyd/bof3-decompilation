#include "internal.h"

/* Absolute bindings are grouped in the adjacent symbols/*.c translation units. */

/* Second local handler table: extern-array binding so `as` expands the
 * indexed relocation with the original `addu $at,$at,$idx` operand order. */
WEAK_SYMBOL_AT(WORLD00_AREA016_D_801F512C, 0x801f512c);
