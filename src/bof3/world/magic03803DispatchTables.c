#include "bof3/bof3.h"

extern void (*D_801EF9A4[])(void);
extern void (*D_801EF9D0[])(void);

/* @source 0x801EF0D0
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its unsigned state byte at offset 1 is scaled by four and that entry of the
 * overlay-local handler-pointer table at 0x801EF9A4 (seven in-image code
 * pointers, entry 0 = 0x801EF114 through entry 6 = 0x801EF884, before the four
 * packed byte words at 0x801EF9C0) is invoked with no arguments. It reads no
 * other state and writes none; the 0x18-byte frame only keeps $ra across the
 * indirect call and the pointer-cell load precedes frame creation. No word in
 * this payload addresses 0x801EF0D0, so the caller is outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchWorkByte1HandlerBmagicMagic03803_801EF0D0, D_801EF9A4);

/* @source 0x801EF8B8
 * @behavior Overlay step dispatch: reads the state byte at +2 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801EF9D0. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_TWO(dispatchStateByte2Tablef9d0BmagicMagic03803_801EF8B8, D_801EF9D0);
