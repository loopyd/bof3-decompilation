#include "bof3/bof3.h"

extern void (*D_801EF89C[])(void);

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
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchWorkByte1HandlerBmagicMagic00903_801EF0DC, D_801EF89C);
