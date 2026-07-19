/* Verified PsyQ/BIOS weak bindings; hand-reviewed. */
#include "bof3/symbols.h"

WEAK_SYMBOL_AT(PushMatrix, 0x80178B78);
WEAK_SYMBOL_AT(MulMatrix2, 0x80178CB8);
WEAK_SYMBOL_AT(SetRotMatrix, 0x80178FD8);
WEAK_SYMBOL_AT(SetTransMatrix, 0x80179068);
WEAK_SYMBOL_AT(RotTrans, 0x80179558);
WEAK_SYMBOL_AT(RotMatrix, 0x80179738);
