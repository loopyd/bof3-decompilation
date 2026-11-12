#include "bof3/bof3.h"

extern u8 D_80181B14;
extern u8 D_80181B13;
extern u8 D_8014496F[];

void func_801C1ABC(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE1C0
 * @behavior Overlay callback reached only through the word at 0x801FE378, the
 * entry 29 of the thirty-entry in-image code-pointer table D_801FE304 that
 * dispatchRecordCallbackByByte7AScenarioScena0200_801FD084 selects with a
 * record's unsigned byte field at offset 0x7A. No in-image jal targets the
 * address, and the body ignores the two arguments that dispatcher passes. It
 * reads the unsigned byte D_80181B14 with `lbu`, clears bit 0 of the
 * field-record flag byte at D_8014496F plus 0xA4 times that index, repeats the
 * same clear for the unsigned byte D_80181B13, then calls the helper at
 * 0x801C1ABC with the constant 4, the same helper with the constant 3, and
 * finally the no-argument helper at 0x801C1630. The 0xA4-byte stride is the
 * shared field-record table this tree anchors at 0x80144968, where 0x8014496F
 * is a record's +7 flag byte; the two index cells' wider roles are not
 * established, so the name keeps the field and the operation rather than a
 * guessed role. The 0x18-byte frame only keeps $ra across the three calls,
 * each of which materialises its own argument. None of these 0xAC bytes is
 * byte-identical to any other emi/scenario payload in this checkout, so no
 * clone applies. The flag-clear block is the shape of the exact sibling
 * clearIndexedFlagBit0AndWorkCountThenResetEffect7ScenarioScena0800_801FE6DC
 * (emi/scenario/scena08/00, one index block and a different tail), and the
 * two-helper tail is the shape of the exact entry 28
 * invokeHelperArgThenHelperScenarioScena0200_801FE198 in this target, so every
 * literal and index cell here comes from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearTwoIndexedRecordFlagBitsThenInvokeHelperTwiceThenHelperScenarioScena0200_801FE1C0(void) {
  s32 offset;

  offset = D_80181B14 * 0xA4;
  D_8014496F[offset] = D_8014496F[offset] & 0xFE;
  offset = D_80181B13 * 0xA4;
  D_8014496F[offset] = D_8014496F[offset] & 0xFE;
  func_801C1ABC(4);
  func_801C1ABC(3);
  func_801C1630();
}
