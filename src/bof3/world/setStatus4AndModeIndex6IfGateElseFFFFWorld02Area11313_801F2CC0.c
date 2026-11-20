#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2CC0
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 *           nonzero, stores 4 into the shared status halfword D_801490A8 and 6
 *           into the shared mode byte at 0x801D4285; when the gate byte is zero
 *           it stores 0xFFFF into D_801490A8 instead and touches nothing else.
 *           Takes no arguments and returns nothing; leaf with no frame, and the
 *           status constant is materialised in $v0 and fills the branch delay
 *           slot.
 *           Shape: a payload cmp of the 68 bytes at 0x801F2CC0 against
 *           out/binaries/emi/world03/area116/13.bin offset 0xCC came back equal
 *           from the first word to the `jr $ra` nop except byte 40, the low
 *           byte of the `j` word at 0x801F2CE8 (0x3F here against 0x42 there),
 *           and that displacement of three words equals the 0x0C offset delta
 *           between the two spans, so the one differing word is an internal
 *           jump target and the two spans are the same body. The recorded-exact
 *           sibling setStatus4AndModeIndex6IfGateElseFFFFWorld03Area11613_801F2CCC
 *           supplied this body's statement order, operand forms, offset
 *           spelling and declaration style; this span's own bytes supplied its
 *           status and mode literals. The five-word handler pointer run at
 *           payload offset 0x1B8 (0x801F2C04, 0x801F2C60, 0x801F2C90,
 *           0x801F2CC0, 0x801F2D04) carries this address as its fourth word, so
 *           the payload installs the body as a table member instead of reaching
 *           it with a `jal`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus4AndModeIndex6IfGateElseFFFFWorld02Area11313_801F2CC0(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 4;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
