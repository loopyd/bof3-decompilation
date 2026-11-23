#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 D_801448EC; /* @source 0x801448EC @kind unknown */

/* @source 0x801F2D7C
 * @behavior Entry 4 of the target handler table at 0x801F37AC. Loads the shared
 * gate byte D_801490C7 into a local, stores 0xFFFF into the shared status
 * halfword D_801490A8, and then stores into the shared progress byte
 * D_801448EC the value 2 while the gate byte is zero, 10 while it is one, and 20
 * for every other value. Takes no arguments, calls nothing and returns nothing;
 * leaf body with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the 0x40-byte span is the fourth handler
 * of the seven-word pointer run this payload publishes at its own 0x801F37AC,
 * which lists 0x801F2C80, 0x801F2CA8, 0x801F2CFC, 0x801F2D54, this address,
 * 0x801F2DBC and 0x801F2E44 in that order. The status store precedes the gate
 * test and the gate byte is read once, which fixes the load, the unconditional
 * 0xFFFF halfword store and then the three-arm selection. The original reaches
 * only one `sb` at the join even though its arms are separate constants, so the
 * arms store the byte directly and the merge happens in the compiler rather than
 * in the source. The three-arm `bnez`/`j`/`bne` walk has no byte-identical twin
 * among the 398 extracted payloads, so this file is seeded from the original
 * bytes and from the two-arm sibling spelling (gate load, unconditional status
 * store, one conditional store of the selected constant) that
 * setStatusFFFFAndByte801448EC0EIfGateElse0CWorld03Area15013_801F2CDC measured
 * exact.
 *
 * Measured shapes for the selection, keeping the gate load and the status store
 * fixed: the arm-local store chain below reached 16/16 instructions; a version
 * that selected into a local and stored once after the chain reached 8/16; a
 * nested conditional expression with `D_801448EC = ...` reached 13/16.
 */
void stateStatusOr20Gate(void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFFu;
  if (gate == 0) {
    D_801448EC = 2;
  } else if (gate == 1) {
    D_801448EC = 10;
  } else {
    D_801448EC = 20;
  }
}
