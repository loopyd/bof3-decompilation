#include "bof3/bof3.h"

extern u8 D_80181B1A;
extern u8 D_8014496F[];
extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FBF18
 * @behavior Overlay per-record callback: its only image word is 0x801FC498,
 * entry 12 of the twenty-two-word callback run at 0x801FC468 (0x801FC468
 * through 0x801FC4BC) that dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C
 * (0x801FB37C) indexes with the unsigned record byte at offset 0x7A, so the
 * overlay selects this entry for the record kind 0x0C, and no in-image jal
 * reaches the address. In source statement order it reads the unsigned byte
 * index D_80181B1A, clears bit 0 of the byte at 0x8014496F plus 0xA4 times that
 * index, re-arms the three-entry overlay effect group for ids 7, 4 and 8
 * through func_80166E88(7, 4, 8, 0), clears the shared live-local work-record
 * count byte D_80146254, runs the per-id effect teardown func_801C187C for ids
 * 7, 4 and 8 in that order, and finishes with the no-argument helper
 * func_801C1630. The 0xA4 stride is the shared field-record table anchored at
 * 0x80144968, where 0x8014496F is a record's +7 flag byte, as the scena01 and
 * scena08 overlays read the same table; 0x8014496F - 0x80144968 is 7 and the
 * multiply the original bytes perform is by 0xA4, and the wider role of both
 * that flag byte and the index cell is not established, so the name keeps the
 * field and the operation rather than a guessed role. It ignores the record
 * pointer and the second argument its dispatcher passes, reads no memory of its
 * own and writes only that indexed flag byte and D_80146254; the 0x18-byte
 * frame only keeps $ra at 0x10 across the five calls, the 7 and 4 arm
 * immediates are materialised in $a0/$a1 before the index load, the 8 follows
 * it in $a2, the zero fourth one sits in the first jal delay slot, each
 * teardown id is materialised in $a0 inside its own jal delay slot, and the
 * func_801C1630 call delay slot carries the nop. Its arm-clear-teardown order
 * and operand forms are those of the neighbour 0x801FBD2C, which carries the
 * arm trio 7, 8, 4 where this body carries 7, 4, 8 and returns without the
 * func_801C1630 tail, so that neighbour is shape evidence and every literal
 * here comes from this span's own bytes; the leading indexed flag clear is the
 * shape of the scena08 body at 0x801FE6DC, which clears the same bit and the
 * same count byte before a single teardown call. No extracted payload holds a
 * byte-identical copy of this 0x90-byte span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearIndexedFlagBit0ArmEffectGroupThenClearWorkRecordsAndResetEffectSlotsThenInvokeHelperScenarioScena1400_801FBF18(void) {
  s32 offset;

  offset = D_80181B1A * 0xA4;
  D_8014496F[offset] = D_8014496F[offset] & 0xFE;
  func_80166E88(7, 4, 8, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(4);
  func_801C187C(8);
  func_801C1630();
}
