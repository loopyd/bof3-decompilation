#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE7AC
 * @behavior Overlay per-record callback: it re-arms the three-entry overlay
 * effect group for ids 7, 2 and 8 through func_80166E88(7, 2, 8, 0), then
 * clears the shared live-local work-record count byte D_80146254, then runs the
 * per-id effect teardown func_801C187C for those same ids 7, 2 and 8 in that
 * order, and finishes with the helper func_801C1630. It ignores the record
 * pointer and the second argument its caller passes, reads no other memory and
 * writes none; the 0x18-byte frame only keeps $ra across the five calls, the
 * three leading arm immediates are materialised in $a0/$a1/$a2 before the first
 * jal with the zero fourth one in that jal's delay slot, each teardown id is
 * materialised in $a0 inside its own jal's delay slot and the trailing helper
 * call carries a nop there. The address is entry 7 - the word at 0x801FE9DC,
 * payload offset 0x7DDC, reading 0x801FE7AC - of the eleven-entry
 * callback-pointer table at 0x801FE9C0 that
 * dispatchRecordCallbackScenarioScena0800_801FE018 selects with the record's
 * unsigned byte at offset 0x7A. It is the arm-clear-teardown sibling of entry 6
 * armEffectGroupThenClearWorkRecordsAndResetEffectSlotsScenarioScena0800_801FE760
 * (0x801FE760), which uses the same call set for the group 7, 2 and 4 and tears
 * down only the ids 7 and 2; this entry arms and then tears down the same three
 * ids, so unlike entry 6 and unlike entry 9
 * armEffectGroupThenResetEffectSlotsScenarioScena0800_801FE840 (0x801FE840) it
 * needs no separately armed group for its teardown list.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectGroupScenarioScena0800_801FE7AC(void) {
  func_80166E88(7, 2, 8, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(2);
  func_801C187C(8);
  func_801C1630();
}
