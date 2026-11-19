#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F365C
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 * nonzero, stores 0x88 into the shared status halfword D_801490A8 and 6 into the
 * shared mode byte at 0x801D4285; when the gate byte is zero it stores 0x86 into
 * D_801490A8 instead and touches nothing else. Takes no arguments and returns
 * nothing; leaf with no frame, and the 0x88 constant is materialised in $v0 and
 * fills the branch delay slot. No `jal` in this payload reaches this address; its
 * only in-payload reference is one pointer word, at 0x801F4A24 inside the
 * script/data blob D_801F3B34, so the entry is a script handler reached only
 * through that table and the blob carries no prototype, which keeps the argument
 * roles structural.
 * Shape: the 68 bytes at 0x801F365C differed from the recorded-exact sibling
 * setStatus60AndModeIndex6IfGateElse5EWorld00Area03713_801F3464 in three bytes
 * only - the low byte of the two stored literals (0x5E and 0x60 there, 0x86 and
 * 0x88 here) and the low byte of the internal `j` immediate, which moved by 0x1F8
 * bytes, the delta between the two spans - so every relative branch, cell
 * reference and store site was identical and the two spans were one body (payload
 * `cmp`). The sibling supplied the statement order, operand forms and declaration
 * style; this span's own bytes supplied the two stored literals and the arm
 * polarity. One payload scan over every extracted overlay found the span exactly
 * once, at its own offset, so no byte-identical twin existed to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus88AndModeIndex6IfGateElse86World00Area03713_801F365C(void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 0x86;
  } else {
    D_801490A8 = 0x88;
    D_801D4285 = 6;
  }
}
