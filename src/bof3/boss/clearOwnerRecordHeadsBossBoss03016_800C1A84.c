#include "bof3/bof3.h"

void clearOwnerRecordHead(u8 id); /* @source 0x800AC934 */

/* @source 0x800C1A84
 * @behavior Second shared battle hook of this overlay: clears the head of the
 * owner records for ids 0, 1 and 2 by calling the main-program helper at
 * 0x800AC934 once per id in ascending order, then returns. The body takes no
 * arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: three `jal 0x800AC934` sites whose delay slots carry
 * `addu $a0,$zero,$zero`, `addiu $a0,$zero,0x1` and `addiu $a0,$zero,0x2`, so
 * the helper is called with ids 0, 1 and 2 in that order, inside the usual
 * -0x18 frame that saves only $ra. The 48 payload bytes are identical to the
 * reviewed exact span at boss023/16 0x800C1A10, so the statement order comes
 * from that evaluated sibling while the callee binding here uses the canonical
 * in-repo name of 0x800AC934.
 */
void clearOwnerRecordHeadsBossBoss03016_800C1A84(void) {
  clearOwnerRecordHead(0);
  clearOwnerRecordHead(1);
  clearOwnerRecordHead(2);
}
