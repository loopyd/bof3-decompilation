#include "bof3/bof3.h"

typedef void (*BmagicMagic01500Handler)(void);

/*
 * Code-pointer run at 0x801F1630, the table this dispatcher indexes:
 * entry 0 = 0x801EFA28, entry 1 = 0x801EFF1C, entry 2 = 0x801F0334,
 * entry 3 = 0x801F0C30. The non-code word 0x02030304 at 0x801F1640 follows the
 * last entry inside the same 0x14-byte run.
 */
extern BmagicMagic01500Handler D_801F1630[]; /* @source 0x801F1630 @kind data */

/* @source 0x801EF9E4
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, its unsigned byte at offset 1 is scaled by four and the selected entry
 * of the four-entry overlay-local code-pointer run at 0x801F1630 (0x801EFA28,
 * 0x801EFF1C, 0x801F0334, 0x801F0C30) is invoked with no arguments, so no
 * argument register is loaded. It reads no other state and writes none; the
 * 0x18-byte frame only keeps $ra across the indirect call, the pointer-cell load
 * precedes frame creation, and the index load, the scaled index and the table
 * load each follow a nop delay slot. No word in this payload addresses
 * 0x801EF9E4, so the caller is outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic01500_801EF9E4(void) {
  D_801F1630[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
