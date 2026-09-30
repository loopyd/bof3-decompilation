#include "bof3/bof3.h"

void func_8015C088(void);

/* @source 0x801FDFEC
 * @behavior Takes the per-record callback's record pointer in $a0 and returns nothing: it runs
 * the shared helper at 0x8015C088 (the address sibling overlays call as the front-end startup
 * helper) and then increments that record's unsigned 16-bit field at offset 0x7E in place. The
 * record pointer is the only value it keeps, and only across the call: it is copied from $a0 into
 * $s0 in the call's delay slot, so the 0x18-byte frame saves $ra and $s0. The address is entry 11,
 * the last of the twelve callback pointers of this overlay's per-record table at 0x801FE6EC,
 * which dispatchRecordCallbackScenarioScena1500_801FDD80 selects with the record's byte at 0x7A.
 * The name reuses this repository's existing wording for both halves of the body: the shared
 * helper at 0x8015C088 is called the front-end startup helper by the exact sibling lifts that
 * invoke it (e.g. src/bof3/scenario/requestPrimaryState0AAndClearSubstateScenarioScena0900_801FC6DC.c)
 * and the in-place halfword at 0x7E is named the same way by
 * incrementWorkHalfword7EScenarioScena1300_801FB390 (0x801FB390 in emi/scenario/scena13/00).
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeStartupThenIncrementRecordHalfword7EScenarioScena1500_801FDFEC(u8 *record) {
  func_8015C088();
  *(u16 *)(record + 0x7E) += 1;
}
