#include "bof3/bof3.h"

typedef void (*BmagicMagic02103Handler)(void);

/* Overlay-local handler-pointer table this dispatcher indexes, based at
 * 0x801EFE04: entry 0 = 0x801EF578, entry 1 = 0x801EF600, entry 2 = 0x801EF750,
 * entry 3 = 0x801EF7A0, the four following functions of this overlay. The run
 * sits inside this overlay's trailing data block, which no map row owns, so
 * the reference is bound by the target's generated undefined-symbol table. */
extern BmagicMagic02103Handler D_801EFE04[]; /* @source 0x801EFE04 @kind data */

/* @source 0x801EF534
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its unsigned state byte at offset 1 is scaled by four and the selected entry
 * of the overlay-local handler run based at 0x801EFE04 (four in-image code
 * pointers, entry 0 = 0x801EF578 through entry 3 = 0x801EF7A0) is invoked with
 * no arguments, so no argument register is loaded. The original does not bound
 * the index before the scaled load. It reads no other state and writes none;
 * the 0x18-byte frame only keeps $ra across the indirect call, and the
 * pointer-cell load precedes frame creation. The 68-byte body is
 * instruction-for-instruction identical to the work-byte-1 dispatchers of
 * emi/bmagic/magic001/00 (0x801EFAB8), emi/bmagic/magic003/03 (0x801EFA9C),
 * emi/bmagic/magic009/03 (0x801EF0DC) and emi/bmagic/magic012/00 (0x801EEFF8),
 * differing only in the table address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic02103_801EF534(void) {
  D_801EFE04[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
