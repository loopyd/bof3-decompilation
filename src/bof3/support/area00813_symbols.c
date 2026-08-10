#include "bof3/world/area00813_internal.h"

/* Absolute bindings are grouped in the adjacent symbols/*.c translation units. */
WEAK_SYMBOL_AT(g_areaWork, 0x1f800044);

/* Shared PsyQ primitive cursor (owned by the main exe; weak-bound here so the
 * overlay resolves to the exe's definition at link time). */
WEAK_SYMBOL_AT(g_PrimCursor, 0x8014598c);
WEAK_SYMBOL_AT(countdown, 0x801f53f4);

/* Local handler table: extern-array binding so `as` expands the indexed
 * relocation with the original `addu $at,$at,$idx` operand order. */
WEAK_SYMBOL_AT(modeHandlerTable, 0x801f4688);
WEAK_SYMBOL_AT(areaStateModeHandlerTable, 0x801f46b0);
WEAK_SYMBOL_AT(D_801F46EC, 0x801f46ec);
