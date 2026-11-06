#include "bof3/bof3.h"

extern u8 D_80181B1A;
extern u8 D_80181B19;
extern u8 D_8014496D[];
extern u8 D_8014496F[];
extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

/* @source 0x801FE10C
 * @behavior Overlay handler reached only indirectly: its address is word 2 of
 * the six-word code-pointer run at 0x801FE35C, the word stored at 0x801FE364,
 * and no in-image jal targets it. That run is the handler table that
 * dispatchCoordinateLookupHandlerScenarioScena0100_801FE03C
 * (0x801FE03C) indexes with the signed byte returned by func_801A7AD8, so
 * this entry runs only if the lookup returns index 2; the dispatched record
 * table holds two identical records, so the reachable indices are 0 and 1 and
 * the shipped data never selects this entry. In source statement order it
 * reads the unsigned byte index D_80181B1A, clears bit 0 of that field
 * record's flag byte at 0x8014496F plus 0xA4 times the index, re-arms the
 * three-entry overlay effect group for the ids 9, 3 and 4 through
 * func_80166E88(9, 3, 4, 0), reads the second unsigned byte index D_80181B19,
 * clears the shared work-record count D_80146254, writes 9 into that field
 * record's byte at 0x8014496D plus 0xA4 times the index, and finally calls the
 * per-id effect helper func_801C187C with id 9. The 0xA4-byte stride is the
 * shared field-record table anchored at 0x80144968, where 0x8014496D is a
 * record's +5 byte and 0x8014496F its +7 flag byte, as the etc/shop overlay
 * reads the same table. The two index cells and the role of the +5 byte are
 * not established beyond that, so the name keeps the field and its operation,
 * not a guessed role. Takes no arguments and returns nothing; the 0x18-byte
 * frame only keeps $ra across the two calls, each of which materialises its
 * own argument constants. This is the same flag-clear/work-count shape as the
 * exact sibling clearIndexedFlagBit0AndWorkCountThenResetEffect7ScenarioScena0800_801FE6DC
 * (0x801FE6DC of emi/scenario/scena08/00), which additionally drops the effect
 * group arming, the second index block and the +5 byte store, so that sibling
 * is shape evidence and the literals here come from the original bytes. No
 * byte-identical copy of these 0xA4 bytes exists in another payload of this
 * checkout.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearIndexedRecordFlagArmEffectGroupThenSetRecordByte5AndResetEffect9ScenarioScena0100_801FE10C(void) {
  s32 offset;

  offset = D_80181B1A * 0xA4;
  D_8014496F[offset] = D_8014496F[offset] & 0xFE;
  func_80166E88(9, 3, 4, 0);
  offset = D_80181B19 * 0xA4;
  D_80146254 = 0;
  D_8014496D[offset] = 9;
  func_801C187C(9);
}
