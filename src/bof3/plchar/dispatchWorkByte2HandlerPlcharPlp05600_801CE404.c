#include "bof3/bof3.h"

typedef void (*PlcharPlp05600WorkStateHandler)(void);

/*
 * Code-pointer run at 0x801D0858, the table this dispatcher indexes: 24 entries
 * inside the overlay's trailing read-only block. Entries 2 and 8 hold
 * 0x801B2D54, a handler outside this payload, and every other entry is a
 * function of this payload (entry 0 = 0x801CE448, entry 1 = 0x801CE628,
 * entry 14 = 0x801CF934, entry 23 = 0x801D02F4).
 */
extern PlcharPlp05600WorkStateHandler func_801D0858[]; /* @source 0x801D0858 @kind data */

/* @source 0x801CE404
 * @behavior Overlay work-byte-2 dispatcher and entry point: reads the
 * scratchpad work object through the pointer slot at 0x1F800044, takes that
 * object's unsigned byte at offset 2, scales it by four and invokes the
 * selected entry of the 24-entry overlay code-pointer run at 0x801D0858 with
 * no arguments, so no argument register is loaded and the callee's return
 * value in $v0 is discarded. The original does not bound the index before the
 * scaled load. It reads no other state and writes none; the pointer-slot load
 * precedes frame creation, the 0x18-byte frame only keeps $ra across the
 * indirect call, and the slot after the index byte and the slot after the
 * table load stay nops. Its only in-image reference is the word 0x801CE404 at
 * 0x801D08BC, entry 1 of the seven-entry code-pointer run at 0x801D08B8, so
 * the overlay loader that starts the payload here is its caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerPlcharPlp05600_801CE404(void) {
  func_801D0858[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
