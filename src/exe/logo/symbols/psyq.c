#include "bof3/context.h"

/* LIBCD */
WEAK_SYMBOL_AT(CdInit, 0x801cfc30);
WEAK_SYMBOL_AT(CdReadSync, 0x801d209c);

/* LIBETC */
WEAK_SYMBOL_AT(PadInit, 0x801cee7c);
WEAK_SYMBOL_AT(StopCallback, 0x801cf1ec);

/* LIBGPU */
WEAK_SYMBOL_AT(SetDispMask, 0x801d4174);
