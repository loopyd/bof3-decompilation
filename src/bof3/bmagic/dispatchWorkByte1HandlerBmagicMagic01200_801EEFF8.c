#include "bof3/bof3.h"

/*
 * Code-pointer table at 0x801EFBB0, the table this dispatcher indexes:
 * entry 0 = 0x801EF03C, entry 1 = 0x801EF080, entry 2 = 0x801EF188,
 * entry 3 = 0x801EF3C4, entry 4 = 0x801EF4B8, entry 5 = 0x801EF080,
 * entry 6 = 0x801EF3C4, entry 7 = 0x801EF4B8, entry 8 = 0x801EF080,
 * entry 9 = 0x801EF3C4, entry 10 = 0x801EF4B8, entry 11 = 0x801EF670,
 * entry 12 = 0x801EF6A4.
 */
extern void (*func_801EFBB0[])(void); /* @source 0x801EFBB0 @kind data */

/* @source 0x801EEFF8
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 is scaled by four and selects an entry of the
 * overlay-local thirteen-entry code-pointer table at 0x801EFBB0 (the entry is
 * read back with lw from that table), and the selected entry is invoked with no
 * arguments. It reads no state other than that byte and writes none; the
 * 0x18-byte frame only keeps $ra across the indirect call. The bytes are
 * identical to the sibling dispatchers of this overlay family that index a
 * different table (emi/bmagic/magic111/03@0x801EEFF8 and
 * emi/bmagic/magic131/03@0x801EF668), apart from the table relocation itself.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic01200_801EEFF8(void) {
  func_801EFBB0[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
