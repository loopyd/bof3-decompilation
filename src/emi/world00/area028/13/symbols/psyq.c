/* Verified PsyQ/BIOS weak bindings; hand-reviewed. */
#include "bof3/symbols.h"

WEAK_SYMBOL_AT(GetTPage, 0x8017A620);
WEAK_SYMBOL_AT(SetDrawMode, 0x8017C2D8);
WEAK_SYMBOL_AT(rand, 0x8017E3D4);
