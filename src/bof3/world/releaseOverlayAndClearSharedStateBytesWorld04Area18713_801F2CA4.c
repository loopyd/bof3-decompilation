#include "bof3/bof3.h"

extern u8 D_801448EB; /* @source 0x801448EB @kind unknown */
extern u8 D_801448EC; /* @source 0x801448EC @kind unknown */
extern u8 D_801448ED; /* @source 0x801448ED @kind unknown */

/* Shared front-end release helper at 0x8015C058: the sibling bodies that carry
 * this span run it immediately before clearing the shared state bytes, and its
 * own row is still `asm`, so its role is reported structurally here.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* @source 0x801F2CA4
 * @behavior Calls the shared front-end release helper at 0x8015C058 and then
 *           writes zero to the three consecutive shared state bytes at
 *           0x801448EB, 0x801448EC and 0x801448ED, in ascending address order.
 *           Takes no arguments and returns nothing; the 0x18-byte frame exists
 *           only to hold the return address across the call, and `$at` carries
 *           a fresh `lui` for each of the three stores.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Provenance: the 0x38-byte span at payload offset 0xA4 is byte-identical
 * (56/56 bytes, `cmp`-equal and equal md5 of the two extracted payload spans)
 * to the exact releaseOverlayAndClearSharedStateBytesWorld03Area13113_801F2F1C
 * row at its own offset 0x31C and to the exact clearUiSelectionState row at
 * emi/etc/commu00/00 offset 0x2A84, including the call word 0x0C057016. Those
 * siblings supplied the statement order, operand forms and declaration style;
 * the three cleared cells and the called helper all sit in the shared main
 * executable outside every payload, so no image-local address is carried over.
 * This span is not a member of the handler pointer run at 0x801F35D8 (that run
 * holds 0x801F2C04, 0x801F2C5C and 0x801F2E28); the state dispatcher at
 * 0x801F2CDC calls it from its own offsets 0x16C and 0x190, which keeps the
 * three words as the shared state tuple rather than scratch.
 */
void releaseOverlayAndClearSharedStateBytesWorld04Area18713_801F2CA4(void) {
  func_8015C058();
  D_801448EB = 0;
  D_801448EC = 0;
  D_801448ED = 0;
}
