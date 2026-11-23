#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u8 D_801448EC;
extern u16 D_801490A8;

/* @source 0x801F2C04
 * @behavior Reads the shared gate byte D_801490C7 and, when that byte is zero,
 *           writes 2 to the shared mode byte D_801448EC and then stores 0xFFFF
 *           into the shared status halfword D_801490A8. The 0xFFFF store is
 *           present on both arms, so the status halfword is written on every
 *           path. Takes no arguments and returns nothing; the body is a leaf
 *           with no frame, and the status constant is materialised in the
 *           branch delay slot and again after the mode store.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The span at payload offset 4 is byte-identical (52/52 bytes, payload cmp) to
 * the already-exact World00Area03313_801F2C04 body, so that sibling supplied
 * the two-arm statement order and the declaration style; the three shared cells
 * and the 0x2/0xFFFF literals come from this payload's own bytes.
 */
void stateSelectModeGate(void) {
  if (D_801490C7 == 0) {
    D_801448EC = 2;
    D_801490A8 = 0xFFFFu;
  } else {
    D_801490A8 = 0xFFFFu;
  }
}
