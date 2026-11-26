#include "bof3/bof3.h"

extern s8 D_801448EB; /* @source 0x801448EB @kind data */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */
extern s8 D_801448ED; /* @source 0x801448ED @kind data */
extern void func_8015C088(void); /* @source 0x8015C088 @kind function */

/* @source 0x801F2C04
 * @behavior Runs the shared front-end startup helper at 0x8015C088 and then
 *           writes 0x2C, 0 and 0x0E to the three consecutive shared byte cells
 *           at 0x801448EB, 0x801448EC and 0x801448ED; takes no arguments and
 *           returns nothing. The 0x18-byte frame exists only to keep $ra across
 *           the call, whose delay slot stays a `nop` because the helper takes
 *           no arguments, and each byte store materialises the 0x8014 page in
 *           $at on its own. The 0x0E constant is materialised in $v0 ahead of
 *           the middle store so that store can write $zero. No in-image `jal`
 *           or pointer word names 0x801F2C04, so this overlay reaches the
 *           handler from outside its payload and its argument roles stay
 *           structural.
 *
 *           Shape: the 0x40-byte span is instruction-identical to the sibling
 *           initArea093State of emi/world02/area093/13, whose own metadata
 *           recorded that span as exact when it was lifted, except for one
 *           word: the `addiu` that loads the third constant (0x0B there, 0x0E
 *           here) at payload offset 0x1C. A scan of the extracted payloads
 *           under out/binaries finds no byte-identical twin, so the clone lever
 *           did not apply and only that sibling's statement order, operand
 *           forms and cell widths were transcribed, with this span's own
 *           constants written from its bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateEntryTupleMode14(void) {
  func_8015C088();
  D_801448EB = 0x2C;
  D_801448EC = 0;
  D_801448ED = 0xE;
}
