#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 D_80146874; /* @source 0x80146874 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

void func_8015C088(void);

/* @source 0x801F2F40
 * @behavior Entry 1 of the handler pointer run the payload holds at 0x801F4168
 * (its word sits at 0x801F416C, between the 0x801F2F18 and 0x801F2F94 entries
 * of the same run). Stores 0xFFFF into the shared status halfword D_801490A8 on
 * every path and, while the shared gate byte D_801490C7 is clear, runs the
 * shared front-end startup helper func_8015C088 and then requests primary scene
 * state 1 with secondary state 0x14 through the shared route bytes D_80146874
 * and D_80146875. The gate is read with `lb` (so the cell is s8) and that load
 * is hoisted above the status store; `bnez` jumps to the epilogue, so the
 * gate-clear path falls through its two byte stores into the frame restore. The
 * 0x18-byte frame only keeps $ra across the helper call and that call's delay
 * slot stays a nop because the helper takes no arguments. Each of the two byte
 * stores materialises the 0x8014 page in $at on its own. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Name evidence: the pointer run at 0x801F4168 lists this address second, so
 * this span is a table entry rather than a `jal` target and its roles stay
 * structural. 0x80146874 and 0x80146875 are the shared primary state byte and
 * secondary sub-state byte the requestPrimaryState* family names that way,
 * func_8015C088 is the shared front-end startup helper the whole world family
 * runs, D_801490C7 is the gate byte the neighbouring setStatusFFFF* handlers of
 * the same run branch on, and D_801490A8 is the shared status halfword they
 * store.
 *
 * Shape evidence: this 0x54-byte span is byte-identical to the exact lift at
 * emi/world04/area192/13 0x801F2CA8, proven by comparing both 0x54-byte payload
 * spans at their own offsets (0x340 here, 0xA8 there); `cmp` reports no
 * differing byte, so this file clones that source's statement order, operand
 * forms and declaration style with no substitution of its own.
 */
void setStatusFFFFAndRequestPrimaryState1Substate14WhenGateClearWorld04Area19113_801F2F40(
    void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_80146874 = 1;
    D_80146875 = 0x14;
  }
}
