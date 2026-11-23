#include "bof3/bof3.h"

/* @source 0x801F2C48
 * @behavior Overlay step handler at index 0 of the local handler table based at
 *           0x801F3FF4, which the dispatcher
 *           callStateTable3World01 indexes with
 *           the state byte at +0x01 of the work record published through the
 *           scratchpad pointer cell 0x1F800044: stores 0xB4 (180) into that
 *           record's byte at +0x09, then stores 1 into the state byte at +0x01,
 *           the index of the handler func_801F2C6C in the same table, so the
 *           next dispatch enters it. The pointer cell is re-read with its own
 *           `lui`/`lw` pair for the second store, which fills the `jr $ra`
 *           delay slot. Takes no arguments, returns nothing and uses no frame.
 *
 * The byte at +0x09 is the countdown the overlay's exact body
 * countdownScratchStateToMode13World01Area06813_801F32CC decrements.
 *
 * Shape evidence: the 0x24-byte body is the same instruction shape as
 * armCountdownByte9To210AndAdvanceWorkByte1World03Area14013_801F46E4 in
 * world03/area140/13, which gated exact and differs only in the stored countdown
 * literal (0xD2 there), so it supplied the statement order and the
 * `SPAD_PTR_TABLE` reload spelling. A scan of every extracted payload under
 * out/binaries found these 36 bytes only at offset 0x48 of this target, so no
 * byte-identical sibling existed to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdownTo180State1(void) {
  u8** slots;

  slots = SPAD_PTR_TABLE(u8);
  slots[0x11][9] = 0xB4;
  slots[0x11][1] = 1;
}
