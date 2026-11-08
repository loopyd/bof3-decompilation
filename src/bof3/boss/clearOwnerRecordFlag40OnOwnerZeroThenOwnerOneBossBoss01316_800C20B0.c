#include "bof3/bof3.h"

void clearOwnerRecordFlag40(u8 id); /* @source 0x800AC844 */

/* @source 0x800C20B0
 * @behavior Second member of this overlay's battle hook trio: clears the 0x40
 * flag of the battle record owned by id 0 through clearOwnerRecordFlag40, then
 * the same flag of the record owned by id 1, and returns. The installer
 * publishBattleHooksBossBoss01316_800C200C stores this address into the shared
 * battle hook slot 0x801463A8, which the battle step loads and calls through
 * jalr, so no jal in this payload names it.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: two `jal clearOwnerRecordFlag40` sites whose delay slots carry
 * `addu $a0, $zero, $zero` and `addiu $a0, $zero, 0x1`, inside a 0x18 byte
 * frame that keeps the return address across both calls. The body is
 * byte-identical to the exact lift invokeHelperArgZeroThenOneBossBoss00716_800C1D50
 * of BIN/BOSS/BOSS007.EMI#16.
 */
void clearOwnerRecordFlag40OnOwnerZeroThenOwnerOneBossBoss01316_800C20B0(void) {
  clearOwnerRecordFlag40(0);
  clearOwnerRecordFlag40(1);
}
