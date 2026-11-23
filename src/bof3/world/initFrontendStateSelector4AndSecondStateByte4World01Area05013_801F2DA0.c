#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F2DA0
 * @behavior Runs the shared front-end startup helper at 0x8015C088, then
 *           writes 4 into the shared front-end state selector byte at
 *           0x801448EB and 4 into the shared second state byte at 0x801448ED,
 *           leaving the middle byte 0x801448EC untouched; takes no arguments
 *           and returns 0. The 0x18-byte frame only keeps $ra across the call
 *           and the delay slot behind that jal stays a nop because the helper
 *           takes no arguments. Both stores write the same constant, so this
 *           span materialises 4 into $v1 once and reuses it for the second
 *           store, while each byte store still materialises the 0x8014 page in
 *           $at on its own. The helper sets flags; its incidental address in
 *           $v0 is not a declared result. This function returns zero.
 *
 * Shape: the 52 bytes at 0x801F2DA0 equal the first 28 bytes of the 56-byte
 * world01 sibling initFrontendStateSelector4AndSecondStateByte7World01Area07413_801F2E94
 * (that payload's 0x294 offset, payload cmp) and then diverge by exactly the
 * sibling's second constant materialisation, addiu $v1,$zero,0x7: this span
 * stores the same 4 twice, so byte 29 of the payload is the first difference
 * and every following word is that sibling's word one instruction later. That
 * sibling carried a recorded exact measurement and supplied the statement
 * order and the plain extern declarations. The helper's void declaration
 * follows its original behavior, not the donor's return-type guess. The 7 in
 * the donor's second store is not reused here.
 * The role of the 4 in the second byte is not established beyond its position.
 * No in-image jal or pointer word addresses 0x801F2DA0, so the overlay reaches
 * this handler from outside its payload.
 * @status partial
 * @match 76.92
 * @residual With the original void helper contract, the stored constant uses v0 instead of v1; size remains 52 bytes.
 */
s32 initFrontendStateSelector4AndSecondStateByte4World01Area05013_801F2DA0(void) {
  func_8015C088();
  D_801448EB = 4;
  D_801448ED = 4;
  return 0;
}
