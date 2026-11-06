#include "bof3/bof3.h"

u8* findFreeWorkRecordAt800E4800World03Area13413_801F4938(void);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer for the byte-6 read and again for the
 * position pair, so the fixed-address cell symbol keeps both loads. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F48B0
 * @behavior Work-record spawn: it asks the overlay's own scan at 0x801F4938
 * for the first free entry of the 0x20-entry, 0x28-byte-stride record table at
 * 0x800E4800 and returns without writing anything when every entry is claimed.
 * Otherwise it seeds that record from the work object published at the
 * scratchpad pointer cell 0x1F800044: the record's lead byte at offset 0 takes
 * the published record's byte at offset 0x06 - the step stamp this overlay's
 * clearWorkTableOnFirstStepArmCountdownStampStepCountAndAdvanceWorkByte1World03Area13413_801F4214
 * writes there, and the lead byte is the claim flag the scan tests - its bytes
 * at 0x01 and 0x02 are cleared, its byte at 0x03 takes 0x80 and its halfword at
 * 0x04 takes 0x80, then the cell is read again and the record's word at 0x08
 * takes the published record's word 0x34 and its word at 0x0C the published
 * word 0x38, the position pair this overlay's selectScratchState* siblings
 * compare through offset 0x34. The remaining words are cleared apart from the
 * word at 0x18, which takes 0x1800; their roles stay described by offset. The
 * original materialises and reloads the cell address once for the byte-6 read
 * and once for the position pair, so the stamp is read through the
 * fixed-address cell index and the pair through a pointer local.
 *
 * Name evidence: the only in-image reference to this address is jal 0x801F48B0
 * at 0x801F42AC inside the unlifted handler func_801F4288, which reaches it
 * only when (published work byte 9 & 3) == 0, i.e. every fourth tick of the
 * countdown byte the same overlay arms with 0x80. The callee is this target's
 * exact lift findFreeWorkRecordAt800E4800World03Area13413_801F4938, whose
 * recorded role is the free-entry scan of that table and whose lead byte is the
 * claim flag, and the shape is the 0x800E4800-table sibling of the scenario
 * overlay's exact claimFreeWorkRecordSeedWorkPositionAndAngleVectorScenarioScena0800_801F802C
 * and seedClaimedWorkRecordFromWorkObjectScenarioScena0800_801F8B50, which seed
 * their records' position words from the same 0x34/0x38 cell pair of the
 * scratch work object.
 *
 * Shape evidence: the position seeding and the "claim then seed" statement
 * order follow those two exact siblings; this target's own literals, field
 * offsets and reload points come from the original bytes.
 *
 * Matching note: the stamp read is the decisive shape. Binding the cell to a
 * pointer local for it (`stamp = work[6]`, or an inline `work[6]` store) let
 * the scheduler lift the first 0x80 literal into the empty branch delay slot
 * and give the cell pointer $v1 instead of $v0, which cost four bytes. Reading
 * the stamp straight through the volatile cell index keeps the load in $v0,
 * the stamp in $v1 and the delay slot as the original's nop; the pointer local
 * then opens only at the position-pair group.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0(void) {
  u8* record;
  u8* work;

  record = findFreeWorkRecordAt800E4800World03Area13413_801F4938();
  if (record == 0) {
    return;
  }
  record[0] = D_1F800044[6];
  record[1] = 0;
  record[2] = 0;
  record[3] = 0x80;
  work = D_1F800044;
  *(u16*)(record + 4) = 0x80;
  *(u32*)(record + 0x08) = *(u32*)(work + 0x34);
  *(u32*)(record + 0x0C) = *(u32*)(work + 0x38);
  *(u32*)(record + 0x10) = 0;
  *(u32*)(record + 0x18) = 0x1800;
  *(u32*)(record + 0x1C) = 0;
  *(u32*)(record + 0x20) = 0;
}
