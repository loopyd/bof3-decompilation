#include "bof3/bof3.h"

/*
 * Code-pointer table at 0x801EFBB0, the table this dispatcher indexes one
 * entry in: entry 0 = 0x801EF03C (this dispatcher), entry 1 = 0x801EF080,
 * entry 2 = 0x801EF188, entry 3 = 0x801EF3C4, entry 4 = 0x801EF4B8,
 * entry 5 = 0x801EF188, entry 6 = 0x801EF3C4, entry 7 = 0x801EF4B8,
 * entry 8 = 0x801EF188, entry 9 = 0x801EF3C4, entry 10 = 0x801EF4B8,
 * entry 11 = 0x801EF670, entry 12 = 0x801EF6A4.
 */
extern void (*func_801EFBB0[])(void); /* @source 0x801EFBB0 @kind data */

/* @source 0x801EF03C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * once, that object's unsigned byte at offset 2 is scaled by four and selects
 * an entry of the overlay-local thirteen-entry code-pointer table at
 * 0x801EFBB0, and the selected entry is invoked with no arguments. The index is
 * offset by one entry because entry 0 of that table is this dispatcher itself:
 * work byte 2 value 0 invokes 0x801EF080, the overlay's first work-byte-2
 * state handler. It reads no other state and writes none; the 0x18-byte frame
 * only keeps $ra across the indirect call, and the pointer-cell load precedes
 * frame creation. The selection sequence is byte-identical to the work-byte-1
 * dispatcher at 0x801EEFF8 of this overlay apart from the work-object byte
 * offset (2 instead of 1) and the table base (+4, the entry offset).
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerBmagicMagic01200_801EF03C(void) {
  func_801EFBB0[SPAD_PTR_SLOT(u8, 0x44u)[2] + 1]();
}
