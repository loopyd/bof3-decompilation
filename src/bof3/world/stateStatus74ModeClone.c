#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F3618
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 * nonzero, stores 0x74 into the shared status halfword D_801490A8 and 6 into the
 * shared mode byte at 0x801D4285; when the gate byte is zero it stores 0xFFFF
 * into D_801490A8 instead and touches nothing else. Takes no arguments and
 * returns nothing; leaf with no frame, and the 0x74 constant is materialised in
 * $v0 and fills the branch delay slot. No `jal` in this payload reaches this
 * address; its only in-payload reference is one pointer word, at 0x801F4A20
 * inside the script/data blob D_801F3B34.
 * Shape: the 68 bytes at 0x801F3618 differ from the recorded-exact sibling
 * stateStatus74ModeGate in exactly one
 * word, the internal `j` immediate at 0x801F3640, whose value moves by 0x44
 * bytes, the delta between the two spans, so every relative branch, literal and
 * cell reference is identical and the two spans are one body (payload `cmp`).
 * The sibling supplies the statement order and declaration style.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatus74ModeClone(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x74;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
