#include "bof3/bof3.h"

/*
 * Code-pointer table at 0x801F0424 in this overlay's trailing read-only block,
 * the table this dispatcher indexes: entry 0 = 0x801EEFA8 and entry 1 =
 * 0x801EF4E4, both functions of this overlay. The words before it in the same
 * block (0x801F0404 through 0x801F0420) are not code pointers.
 */
extern void (*D_801F0424[])(void); /* @source 0x801F0424 @kind data */

/* @source 0x801EEF64
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is loaded
 * before frame creation, that object's unsigned byte at offset 1 is scaled by
 * four and selects an entry of the two-entry code-pointer table at 0x801F0424
 * (entry 0 = 0x801EEFA8, entry 1 = 0x801EF4E4), and the selected entry is
 * invoked with no arguments, so no argument register is loaded. The original
 * does not bound the index before the scaled load. It reads no other state and
 * writes none; the 0x18-byte frame only keeps $ra across the indirect call, and
 * the load slot after the index byte and after the table load stay nops. No
 * word in this payload addresses 0x801EEF64, so the caller is outside the
 * overlay. The 68-byte body is instruction-for-instruction identical to the
 * work-byte-1 dispatchers of emi/bmagic/magic012/00 (0x801EEFF8) and
 * emi/bmagic/magic131/03 (0x801EF668), differing only in the %lo table-address
 * immediate of the lw.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic03903_801EEF64(void) {
  D_801F0424[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
