/* Verified PsyQ/BIOS weak bindings; hand-reviewed. */
#include "bof3/symbols.h"

WEAK_SYMBOL_AT(GetTPage, 0x8017A620);
WEAK_SYMBOL_AT(GetClut, 0x8017A6F0);
WEAK_SYMBOL_AT(SetSprt, 0x8017AA1C);
WEAK_SYMBOL_AT(SetDrawMode, 0x8017C2D8);
WEAK_SYMBOL_AT(strcat, 0x8017E364);
WEAK_SYMBOL_AT(rand, 0x8017E3D4);
WEAK_SYMBOL_AT(sprintf, 0x8017E3F4);
