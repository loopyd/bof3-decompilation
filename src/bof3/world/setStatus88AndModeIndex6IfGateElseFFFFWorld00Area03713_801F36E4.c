#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F36E4
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 * nonzero, stores 0x88 into the shared status halfword D_801490A8 and 6 into the
 * shared mode byte at 0x801D4285; when the gate byte is zero it stores 0xFFFF
 * into D_801490A8 instead and touches nothing else. Takes no arguments and
 * returns nothing; leaf with no frame, and the 0x88 constant is materialised in
 * $v0 and fills the branch delay slot. No `jal` in this payload reaches this
 * address, and its only in-payload reference is the little-endian handler word
 * at 0x801F4A34 inside the script/data blob D_801F3B34, so the entry is a script
 * handler reached only through that table and the blob carries no prototype,
 * which keeps the argument roles structural.
 * Shape: the 68 bytes at 0x801F36E4 were compared with the lifted sibling
 * setStatus88AndModeIndex6IfGateElseFFFFWorld00Area03713_801F36A0 by a payload
 * `cmp` and differed in one word only, the internal `j` immediate, whose
 * displacement of 0x44 bytes is exactly the delta between the two spans, so
 * every relative branch, cell reference, stored literal and store site was
 * identical and the two spans were one body. That sibling, whose own row is a
 * lifted boundary measured exact, supplied the statement order, operand forms
 * and declaration style; this span's own bytes supplied its `j`. The sibling
 * pair setStatus60AndModeIndex6IfGateElseFFFFWorld00Area03713_801F3508 and its
 * 0x801F354C twin, which relate to each other the same way, supplied the
 * duplicate naming form: one descriptive name per body with the address suffix
 * kept distinct.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus88AndModeIndex6IfGateElseFFFFWorld00Area03713_801F36E4(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x88;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
