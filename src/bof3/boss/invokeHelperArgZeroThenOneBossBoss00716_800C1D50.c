#include "bof3/bof3.h"

void func_800AC844(s32 arg0);

/* @source 0x800C1D50
 * @behavior Thin overlay wrapper: calls the helper at 0x800AC844 with
 * argument zero, then with argument one, and returns.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: two `jal func_800AC844` sites whose delay slots carry
 * `addu $a0, $zero, $zero` and `addiu $a0, $zero, 0x1`. The callee at
 * 0x800AC844 is the in-repo lift `clearOwnerRecordFlag40`.
 */
void invokeHelperArgZeroThenOneBossBoss00716_800C1D50(void) {
  func_800AC844(0);
  func_800AC844(1);
}
