/* Verified PsyQ/BIOS weak bindings; hand-reviewed. */
#include "bof3/symbols.h"

WEAK_SYMBOL_AT(GetTPage, 0x8017A620);
WEAK_SYMBOL_AT(SetSemiTrans, 0x8017A904);
WEAK_SYMBOL_AT(SetDrawMode, 0x8017C2D8);
