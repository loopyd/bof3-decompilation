#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FB5F8
 * @behavior Overlay per-record callback: it runs the shared front-end startup
 * helper func_8015C088 (0x8015C088), requests primary scene state 7 by storing
 * 7 in the shared primary state byte D_80146874 (0x80146874), requests
 * secondary sub-state 0x1E by storing 0x1E in the shared secondary sub-state
 * byte D_80146875 (0x80146875), and then increments the record's unsigned
 * 16-bit field at offset 0x7E in place. It takes the record pointer in $a0,
 * ignores the second argument its dispatcher passes, and returns nothing. The
 * record pointer is the only value kept across the call: it is copied from $a0
 * into $s0 in that jal's delay slot, so the 0x18-byte frame saves $s0 and $ra,
 * and each shared-byte store materialises the 0x8014 page in $at on its own
 * while both store constants and the incremented halfword share $v0. No
 * in-image jal reaches the address; its only image word is 0x801FC484, entry 7
 * of the twenty-two-word overlay callback run at 0x801FC468 (0x801FC468
 * through 0x801FC4BC) that dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C
 * (0x801FB37C) indexes with the unsigned record byte at offset 0x7A. Its
 * neighbour requestPrimaryState5Substate1EAndIncrementRecordHalfword7EScenarioScena1400_801FB5A8
 * (0x801FB5A8, entry 6) is the same startup-and-request shape for primary state
 * 5, and the exact sibling lift
 * invokeStartupThenIncrementRecordHalfword7EScenarioScena1500_801FDFEC
 * (0x801FDFEC in emi/scenario/scena15/00) carries the same startup call and
 * 0x7E halfword increment, so both halves of the name follow existing
 * repository wording. The 0x7E halfword role beyond "record counter" is not
 * established, so the name stays offset-anchored for it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState7Substate1EAndIncrementRecordHalfword7EScenarioScena1400_801FB5F8(u8 *record) {
  func_8015C088();
  D_80146874 = 7;
  D_80146875 = 0x1E;
  *(u16 *)(record + 0x7E) += 1;
}
