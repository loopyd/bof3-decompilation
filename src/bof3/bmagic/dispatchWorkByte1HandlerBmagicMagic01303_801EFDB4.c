#include "bof3/bof3.h"

/*
 * Code-pointer run at 0x801F03D8, the table this dispatcher indexes: entry 0 =
 * 0x801EFDF8, entry 1 = 0x801EFE80, entry 2 = 0x801EFF88, the three handlers
 * that follow this dispatcher inside the overlay. The words from 0x801F03E4
 * onward are zero.
 */
extern void (*D_801F03D8[])(void); /* @source 0x801F03D8 @kind data */

/* @source 0x801EFDB4
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its unsigned byte at offset 1 is scaled by four and selects an entry of the
 * overlay-local three-entry code-pointer run at 0x801F03D8 (entry 0 =
 * 0x801EFDF8, entry 1 = 0x801EFE80, entry 2 = 0x801EFF88), and that entry is
 * invoked with no arguments. It reads no state other than that byte and writes
 * none; the 0x18-byte frame only keeps $ra across the indirect call, the
 * pointer-cell load precedes frame creation, and the lbu and the table lw are
 * each followed by a nop. No word in this payload addresses 0x801EFDB4, so the
 * caller is outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic01303_801EFDB4(void) {
  D_801F03D8[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
