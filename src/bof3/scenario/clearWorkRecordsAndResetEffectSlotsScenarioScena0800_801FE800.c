#include "bof3/bof3.h"

extern u8 D_80146254;

void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE800
 * @behavior Overlay per-record callback: it clears the shared live-local
 * work-record count byte D_80146254 to zero, then runs the per-id teardown
 * func_801C187C for ids 7, 8 and 4 in that order and finishes with the helper
 * func_801C1630. It ignores the record pointer and the second argument its
 * caller passes, reads no other memory and writes none; the 0x18-byte frame
 * only keeps $ra across the four calls, and each call's argument is materialised
 * in $a0 inside that jal's delay slot. The address is entry 8 (the word reading
 * 0x801FE800 at 0x801FE9E0) of the eleven-entry callback-pointer table at
 * 0x801FE9C0 that dispatchRecordCallbackScenarioScena0800_801FE018 selects with
 * the record's unsigned byte at offset 0x7A. The three ids it tears down are the
 * three-entry overlay effect group of its immediate neighbour at 0x801FE840
 * (entry 9, armEffectGroupThenResetEffectSlotsScenarioScena0800_801FE840), which
 * arms exactly ids 7, 8 and 4 through func_80166E88(7, 8, 4, 0) and then calls
 * this address - its single in-image jal, at 0x801FE85C. The shape is the same
 * reset sequence the scena18 sibling clearWorkRecordsAndResetEffectSlots_scena18
 * (0x801F6D04) runs for ids 0, 3 and 4 behind its own func_80166E88 call, and
 * entries 3, 6 and 7 of the same callback table (0x801FE698, 0x801FE760,
 * 0x801FE7AC) call func_801C1630 as well.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkRecordsAndResetEffectSlotsScenarioScena0800_801FE800(void) {
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(8);
  func_801C187C(4);
  func_801C1630();
}
