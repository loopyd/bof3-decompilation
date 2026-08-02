#include "internal.h"

/* Absolute bindings are grouped in the adjacent symbols/*.c translation units. */
WEAK_SYMBOL_AT(D_1F800044, 0x1f800044);

/* Shared PsyQ primitive cursor (owned by the main exe; weak-bound here so the
 * overlay resolves to the exe's definition at link time). */
WEAK_SYMBOL_AT(D_8014598C, 0x8014598c);
WEAK_SYMBOL_AT(D_801F53F4, 0x801f53f4);
