#include "bof3/bof3.h"

void clearOwnerRecordHead(u8 id); /* @source 0x800AC934 */

/* @source 0x800C1A10
 * @behavior Second shared battle hook of this overlay: clears the head of the
 * owner records for ids 0, 1 and 2 by calling the main-program helper at
 * 0x800AC934 once per id in ascending order, then returns. The body takes no
 * arguments and returns no value; the overlay installer func_800C1984
 * publishes it into the 0x801463A8 hook slot, which the sibling installers of
 * this archive family fill with the same thin helper wrapper.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: three `jal 0x800AC934` sites whose delay slots carry
 * `addu $a0,$zero,$zero`, `addiu $a0,$zero,0x1` and `addiu $a0,$zero,0x2`, so
 * the helper is called with ids 0, 1 and 2 in that order. The callee is the
 * in-repo exact lift clearOwnerRecordHead at 0x800AC934, which clears the five
 * leading bytes of the record selected by its `u8 id` argument. The
 * prologue/epilogue pair is the one
 * invokeHelperArgZeroThenOneBossBoss00716_800C1D50 carries for the same shape
 * with two calls against the sibling callee 0x800AC844; this function adds the
 * third call and nothing else. The 48 bytes are identical to the unreleased
 * span at boss030/16 0x800C1A84, whose row is still `asm` and so supplies no
 * source to clone.
 */
void clearOwnerRecordHeadsBossBoss02316_800C1A10(void) {
  clearOwnerRecordHead(0);
  clearOwnerRecordHead(1);
  clearOwnerRecordHead(2);
}
