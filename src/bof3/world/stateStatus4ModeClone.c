#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2D04
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read and,
 *           while that byte is nonzero, stores 4 into the shared status halfword
 *           D_801490A8 and 6 into the shared mode byte at 0x801D4285; when the
 *           gate byte is zero it stores 0xFFFF into D_801490A8 instead and
 *           writes nothing else. Takes no arguments and returns nothing; leaf
 *           with no frame, and the status constant is materialised in $v0 and
 *           fills the branch delay slot.
 *           Shape: a payload cmp of the 68 bytes at 0x801F2D04 against the
 *           recorded-exact sibling
 *           setStatus4AndModeIndex6IfGateElseFFFFWorld02Area11313_801F2CC0 in
 *           the same payload came back equal from the first word to the
 *           `jr $ra` nop except byte 40, the low byte of this span's `j` word at
 *           0x801F2D2C (0x50 here against 0x3F there), and that displacement
 *           equals the 0x44 bytes between the two spans, so the single differing
 *           word is an internal jump target and the two spans are one body read
 *           at this address. The sibling supplied the statement order, operand
 *           forms, offset spelling and declaration style; this span's own bytes
 *           confirm the 4, 6 and 0xFFFF literals.
 *           The five-word handler pointer run at payload offset 0x1B8
 *           (0x801F2C04, 0x801F2C60, 0x801F2C90, 0x801F2CC0, 0x801F2D04) carries
 *           this address as its fifth word, and no `jal` word in the payload
 *           targets 0x801F2D04, so the payload installs the body as a table
 *           member rather than reaching it with a call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatus4ModeClone(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 4;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
