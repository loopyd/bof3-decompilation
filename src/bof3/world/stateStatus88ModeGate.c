#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F36A0
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 * nonzero, stores 0x88 into the shared status halfword D_801490A8 and 6 into the
 * shared mode byte at 0x801D4285; when the gate byte is zero it stores 0xFFFF
 * into D_801490A8 instead and touches nothing else. Takes no arguments and
 * returns nothing; leaf with no frame, and the 0x88 constant is materialised in
 * $v0 and fills the branch delay slot. No `jal` in this payload reaches this
 * address; its only in-payload reference is one handler word, at 0x801F4A30
 * inside the script/data blob D_801F3B34, so the entry is a script handler
 * reached only through that table and the blob carries no prototype, which keeps
 * the argument roles structural.
 * Shape: the 68 bytes at 0x801F36A0 differed from the recorded-exact sibling
 * stateStatus74ModeGate in two words
 * only - the stored literal (0x74 there, 0x88 here) and the internal `j`
 * immediate, which moved by 0xCC bytes, the delta between the two spans - so
 * every relative branch, cell reference and store site was identical and the two
 * spans were one body (payload `cmp`). The sibling supplied the statement order,
 * operand forms and declaration style; this span's own bytes supplied the stored
 * literal. The same comparison against func_801F36E4, the next row, differed in
 * that `j` immediate alone, so it was a second proof of the body; its own row was
 * still `asm`, which left no source to clone, so this span was seeded from the
 * lifted sibling above.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatus88ModeGate(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x88;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
