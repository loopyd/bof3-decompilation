/* Verified PsyQ/BIOS weak bindings; hand-reviewed. */
#include "bof3/symbols.h"

WEAK_SYMBOL_AT(TermPrim, 0x8017A620);
WEAK_SYMBOL_AT(strcat, 0x8017E364);
WEAK_SYMBOL_AT(rand, 0x8017E3D4);
WEAK_SYMBOL_AT(sprintf, 0x8017E3F4);
