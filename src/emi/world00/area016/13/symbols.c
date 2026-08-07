#include "internal.h"

/* Absolute bindings are grouped in the adjacent symbols/*.c translation units. */

/* Local handler tables: extern-array bindings so `as` expands the
 * indexed relocation with the original `addu $at,$at,$idx` operand order. */
WEAK_SYMBOL_AT(WORLD00_AREA016_D_801F5114, 0x801f5114);
WEAK_SYMBOL_AT(WORLD00_AREA016_D_801F511C, 0x801f511c);
WEAK_SYMBOL_AT(WORLD00_AREA016_D_801F512C, 0x801f512c);
