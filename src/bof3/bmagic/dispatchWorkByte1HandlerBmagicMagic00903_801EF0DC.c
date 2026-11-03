#include "bof3/bof3.h"

typedef void (*BmagicMagic00903Handler)(void);

/*
 * Handler-pointer table at 0x801EF89C, the table this dispatcher indexes:
 * entry 0 = 0x801EF120, entry 1 = 0x801EF6B0, entry 2 = 0x801EF198,
 * entry 3 = 0x801EF2D4, entry 4 = 0x801EF340. The non-code words at
 * 0x801EF8B0 follow the last entry inside the same 0x38-byte run.
 */
extern BmagicMagic00903Handler D_801EF89C[];

/* @source 0x801EF0DC
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its unsigned state byte at offset 1 is scaled by four and that entry of the
 * overlay-local handler-pointer table at 0x801EF89C (five in-image code
 * pointers, entry 0 = 0x801EF120 and entry 4 = 0x801EF340) is invoked with no
 * arguments. It reads no other state and writes none; the 0x18-byte frame only
 * keeps $ra across the indirect call and the pointer-cell load precedes frame
 * creation. No word in this payload addresses 0x801EF0DC, so the caller is
 * outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic00903_801EF0DC(void) {
  D_801EF89C[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
