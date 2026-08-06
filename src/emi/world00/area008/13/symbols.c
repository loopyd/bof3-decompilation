#include "internal.h"

/* Absolute bindings are grouped in the adjacent symbols/*.c translation units. */
WEAK_SYMBOL_AT(g_world00_area008_work, 0x1f800044);

/* Shared PsyQ primitive cursor (owned by the main exe; weak-bound here so the
 * overlay resolves to the exe's definition at link time). */
WEAK_SYMBOL_AT(D_8014598C, 0x8014598c);
WEAK_SYMBOL_AT(world00_area008_countdown, 0x801f53f4);

/* Local handler table: extern-array binding so `as` expands the indexed
 * relocation with the original `addu $at,$at,$idx` operand order. */
WEAK_SYMBOL_AT(world00_area008_mode_handlerTable, 0x801f4688);
