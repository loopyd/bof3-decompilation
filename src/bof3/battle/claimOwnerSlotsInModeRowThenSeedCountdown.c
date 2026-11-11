#include "bof3/battle/battle15_internal.h"

extern u8 D_801CD78E[][4]; /* @source 0x801CD78E @kind table */
extern void claimOwnerSlotInitLocalPanelRecordResetWorkAndAdvanceCount(
    u8 slot, u8 index, u8 class_id); /* @source 0x800AC01C */

/* @source 0x800ABF5C
 * @behavior Clears the initialised record counter D_801462F2, resolves the
 *   party owner row for the battle mode byte D_801462EA by taking the byte of
 *   the 4-byte row D_801CD78E[D_801462EA] as a row index into the 9-byte rows
 *   rooted at the fixed work-area base 0x800E4000, and walks that row's eight
 *   leading bytes: for every byte that is not the empty marker 0xFF it claims
 *   the owner work record of that slot through
 *   claimOwnerSlotInitLocalPanelRecordResetWorkAndAdvanceCount with the slot
 *   index, the number of records claimed so far and the row byte as the class
 *   id that pass forwards to func_800AC218 and func_800AD0A8. After the walk it
 *   copies the counter D_801462F2 into D_801462F3, the countdown that
 *   clearBattlerFlagsAndDecrement462F3 decrements as each battler finishes.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Levers: analysis data-candidates reports no data span in this target, and the
 * span opens with a stack prologue and closes with jr $ra, so the data lever is
 * declined. Carve is ineligible: the asm row starts at its own declared entry
 * and ends at the next declared entry. Clone is declined: a masked scan of the
 * 396 extracted payloads under out/binaries for these 0xC0 bytes, with the one
 * jal word masked, hits only this span's own offset in its own payload.
 *
 * Shape evidence: the body is transcribed from the exact sibling
 * initBattlerRecordsForActiveSlotsThenSeedCountdown (0x800ABC30), which walks
 * eight slots of one record table and seeds the same D_801462F3 countdown; it
 * supplies the statement order, the u8 loop locals and the store order, while
 * this span's own bytes supply the fixed 0x800E4000 row base, the row stride 9,
 * the 0xFF empty marker and the callee. Two spellings came from this span's own
 * words: the row base is a fixed address because the original materialises it
 * with lui plus ori, while a symbol address compiles to lui plus addiu
 * (measured 42/48 with a D_ cell there), and the row byte is an argument to the
 * callee because the original loads it into $a2, the third argument register,
 * and the call site then sets nothing beyond $a0 and $a1; nine spellings that
 * let the byte die at the compare kept it in $v0 and held at 46/48, and passing
 * it as the class id measured 48/48. The callee's own exact body saves $a2 and
 * forwards it to func_800AC218 and func_800AD0A8, so those words are its
 * argument setup.
 */
void claimOwnerSlotsInModeRowThenSeedCountdown(void) {
  u8 *row;
  u8 i;
  u8 count;

  D_801462F2 = 0;
  row = PSX_PTR(u8, 0x800E4000u) + D_801CD78E[D_801462EA][0] * 9;

  i = 0;
  count = 0;
  while (i < 8) {
    if (row[i] != 0xFF) {
      claimOwnerSlotInitLocalPanelRecordResetWorkAndAdvanceCount(i, count,
                                                                 row[i]);
      count++;
    }
    i++;
  }
  D_801462F3 = D_801462F2;
}
