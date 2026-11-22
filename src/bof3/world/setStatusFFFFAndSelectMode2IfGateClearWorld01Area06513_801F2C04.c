#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u8 D_801448EC;
extern u16 D_801490A8;

/* @source 0x801F2C04
 * @behavior Reads the shared gate byte D_801490C7 and, while it is zero, writes
 *           2 to the shared mode byte D_801448EC before storing 0xFFFF into the
 *           shared status halfword D_801490A8. The 0xFFFF store sits on both
 *           arms, so the status halfword is written on every path; the original
 *           materialises that constant in the branch delay slot and again after
 *           the mode store. Takes no arguments and returns nothing; the body is
 *           a leaf with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 52-byte span at payload offset 4 was measured byte-identical to the row
 * of setStatusFFFFAndSelectMode2IfGateClearWorld03Area15113_801F2C04
 * (emi/world03/area151/13) with a payload cmp and a clone dry-run, and that
 * exact sibling supplied the two-arm statement order and the declaration style.
 * The three shared cells and the 2/0xFFFF literals come from this payload's own
 * bytes.
 */
void setStatusFFFFAndSelectMode2IfGateClearWorld01Area06513_801F2C04(void) {
  if (D_801490C7 == 0) {
    D_801448EC = 2;
    D_801490A8 = 0xFFFFu;
  } else {
    D_801490A8 = 0xFFFFu;
  }
}
