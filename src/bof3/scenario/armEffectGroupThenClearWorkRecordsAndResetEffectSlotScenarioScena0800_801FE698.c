#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE698
 * @behavior Overlay per-record callback: it first re-arms the overlay effect
 * group for the ids 7, 2 and 0x0A through func_80166E88(7, 2, 0x0A, 0), then
 * clears the shared live-local work-record count byte D_80146254, then runs the
 * per-id effect teardown func_801C187C with the constant id 0x0A, and finishes
 * with the helper func_801C1630. It ignores the record pointer and the second
 * argument its caller passes, reads no other memory and writes none; the
 * 0x18-byte frame only keeps $ra across the four calls, the three leading
 * argument immediates are materialised in $a0/$a1/$a2 before the first jal and
 * the last one in that jal's delay slot. The address is entry 3 - the word at
 * 0x801FE9CC, payload offset 0x7DCC, reading 0x801FE698 - of the eleven-entry
 * callback-pointer table at 0x801FE9C0 that
 * dispatchRecordCallbackScenarioScena0800_801FE018 selects with the record's
 * unsigned byte at offset 0x7A. It is the immediate neighbour of the two other
 * callbacks in that table that both arm an overlay effect group and then clear
 * D_80146254: entry 8 clearWorkRecordsAndResetEffectSlotsScenarioScena0800_801FE800
 * (0x801FE800) tears down ids 7, 8 and 4, and entry 9
 * armEffectGroupThenResetEffectSlotsScenarioScena0800_801FE840 (0x801FE840) arms
 * those same three ids through func_80166E88(7, 8, 4, 0); entries 6 and 7 of the
 * table (0x801FE760, 0x801FE7AC) and entry 5 (0x801FE740) use the same helper
 * set.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenClearWorkRecordsAndResetEffectSlotScenarioScena0800_801FE698(void) {
  func_80166E88(7, 2, 0xA, 0);
  D_80146254 = 0;
  func_801C187C(0xA);
  func_801C1630();
}
