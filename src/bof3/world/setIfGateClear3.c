#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u8 D_801448EC;
extern u16 D_801490A8;

/* @source 0x801F2C5C
 * @behavior Reads the shared gate byte D_801490C7 and, when that byte is zero,
 *           writes 2 to the shared mode byte D_801448EC and then stores 0xFFFF
 *           into the shared status halfword D_801490A8. The 0xFFFF store is
 *           present on both arms, so the status halfword is written on every
 *           path. Takes no arguments and returns nothing; the body is a leaf
 *           with no frame, and the status constant is materialised in the
 *           branch delay slot and again after the mode store. Entry 1 of the
 *           pointer run at 0x801F35D8 in this payload (word 0x801F35DC),
 *           between the 0x801F2C04 handler and 0x801F2E28; no `jal` names this
 *           body, so it is reached only through that run and its parameter
 *           roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Provenance: the span at payload offset 0x5C is byte-identical (52/52 bytes,
 * payload cmp of the two extracted entries) to the exact
 * setStatusFFFFAndSelectMode2IfGateClearWorld03Area15113_801F2C04 row at its
 * own offset 4, which supplied the two-arm statement order and the declaration
 * style. The three shared cells (main-executable addresses outside both
 * payloads) and the 0x2/0xFFFF literals were re-read from this payload's own
 * bytes and agree with the seed.
 */
void setIfGateClear(void) {
  if (D_801490C7 == 0) {
    D_801448EC = 2;
    D_801490A8 = 0xFFFFu;
  } else {
    D_801490A8 = 0xFFFFu;
  }
}
