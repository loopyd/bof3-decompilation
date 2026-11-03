#include "bof3/bof3.h"

typedef void (*BmagicMagic00100Handler)(void);

/* Overlay-local handler-pointer run this dispatcher indexes, based at
 * 0x801F04B8: its single in-image word is 0x801EFAFC, the next function of
 * this overlay. The word immediately before the run, 0x801EF01C at 0x801F04B4,
 * is the first of the two entries the sibling dispatcher at 0x801EEFD8 indexes
 * through its own base 0x801F04B4. Both runs sit in this overlay's trailing
 * data block, which no map row owns, so the reference is bound by the target's
 * generated undefined-symbol table. */
extern BmagicMagic00100Handler D_801F04B8[]; /* @source 0x801F04B8 @kind data */

/* @source 0x801EFAB8
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, that object's unsigned byte at offset 1 is scaled by four and the
 * selected entry of the overlay-local handler run based at 0x801F04B8 (its
 * only in-image code pointer is 0x801EFAFC at index 0) is invoked with no
 * arguments, so no argument register is loaded. It reads no other state and
 * writes none; the 0x18-byte frame only keeps $ra across the indirect call, the
 * pointer-cell load precedes frame creation, and the index load, the scaled
 * index and the table load each follow a nop delay slot. The 68-byte body is
 * instruction-for-instruction identical to the sibling dispatcher at
 * 0x801EEFD8 of this overlay and to the work-byte-1 dispatcher of
 * emi/bmagic/magic009/03 (0x801EF0DC), differing only in the %lo table-address
 * immediate of the lw.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic00100_801EFAB8(void) {
  D_801F04B8[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
