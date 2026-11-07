#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Table of 0x20 records of 0x2C bytes at 0x801D3B28, directly after the
 * 0x10-entry 0x18-byte-stride record table at 0x801D39A8 and inside the same
 * Splat `textbin` row. It is zero on disc and filled at runtime: func_801D1F68
 * seeds one record from the scratchpad work object's position triple at +0x34
 * and its random angle fields, and stores the halfword pair at +0x28 and +0x2A
 * that this function clears for the records past the first 0x10. */
extern u8 D_801D3B28[]; /* @source 0x801D3B28 @kind data */

void func_801D147C(u8* origin, s16 x, s32 flags);
void func_801D1F68(u8* record);
void clearRecordActiveFlagsAt801D39A8ScenarioSce15ef100_801D199C(void);

/* @source 0x801D0DD8
 * @behavior Entry 3 of the overlay's step-handler code-pointer run at
 * 0x801D3918 (D_801D3918), which the overlay entry func_801D0C04 selects with
 * the unsigned work byte at offset +0x1 of the scratchpad work object published
 * at pointer slot 0x1F800044 and calls with no arguments and no result
 * consumed. It subtracts 6 from the unsigned halfword at +0x2E of that object
 * and stores the truncated halfword back, adds 0xC0000 to the origin word at
 * +0x6C, and hands the overlay helper at 0x801D147C the object's origin block
 * at +0x64, the new halfword read as a signed halfword, and the flag mask 6. It
 * then ticks the object's countdown byte at +0x9 down by one; when that byte
 * reaches zero it re-arms it with 0x20, clears the frame byte at +0x6, advances
 * the work byte at +0x1 by one, clears the active flags of the record table at
 * 0x801D39A8, copies the origin triple at +0x64, +0x68 and +0x6C into the
 * position triple at +0x34, +0x38 and +0x3C, re-seeds the first 0x10 of the
 * 0x20 0x2C-byte-stride records at 0x801D3B28 from that position triple, and
 * clears the halfword pair at +0x28 and +0x2A of the remaining 0x10 records.
 * It takes no arguments, returns nothing and its 0x20-byte frame keeps $ra at
 * 0x18, the record cursor at 0x14 and the record counter at 0x10 across the
 * three calls.
 *
 * Evidence: the run word at 0x801D3924 names 0x801D0DD8, and the words
 * 0x801D391C, 0x801D3920, 0x801D3928, 0x801D392C and 0x801D3930 name
 * 0x801D0CC0, 0x801D0D54, 0x801D0F10, 0x801D0FEC and 0x801D106C, so this body
 * is element 3 of the run whose base 0x801D3918 the overlay entry loads after
 * `sll $v0,$v0,2` of the work byte at +0x1. The helper's arguments are the ones
 * the exact sibling func_801D0D54 records for 0x801D147C: origin block, signed
 * halfword at +0x2E and the flag mask 6. The record init helper func_801D1F68
 * has two in-payload call sites, the walk in this body (jal at 0x801D0EB4) and
 * func_801D106C (jal at 0x801D11F8), and the exact lift
 * clearRecordActiveFlagsAt801D39A8ScenarioSce15ef100_801D199C names this body
 * as its only caller (jal at 0x801D0E80), which is the table walker it records.
 *
 * Shape evidence: the head and the countdown tail are the statement sequence of
 * the exact sibling func_801D0D54 in this target, with this body's own -6
 * advance, 0xC0000 origin-word step and 0x20 re-arm; the `lhu` / `addiu` /
 * `sh` / `sll` / `sra` advance through halfword +0x2E is the form the exact
 * lift func_801D0CC0 makes with +0x20. The origin-into-position copy with three
 * named word locals is the exact lift func_801D0F10's spelling of the same
 * triple copy. The two record walks are the do/while pointer-and-unsigned-byte
 * counter shape of the exact lifts clearRecordActiveFlagsAt801D39A8ScenarioSce15ef100_801D199C
 * and clearWorkTableAt800E4800ScenarioSce15ef200_801D15B0, including the
 * `andi ...,0xFF` that a `u8` counter's bound test emits, and the second walk
 * reaches its halfword pair through the cursor the original derives at +0x2A
 * (`sh $zero,-0x2($a0)` then `sh $zero,0x0($a0)`, stepping +0x2C).
 *
 * Shape decisions measured on this body: the head's origin-word update is
 * written before the halfword advance, which puts its load ahead of the
 * `lhu` and lets the delay-slot filler place that store in the `jal` slot the
 * original shows. The guard block addresses the cell through its own pointer
 * local (`motion`) instead of the head's, because one shared local made the
 * single quantity take one register across the whole function and cost the
 * original's `$v1` in the head and `$v0` in the guard block. Both record walks
 * carry the counter in a `u8`; the guard block's walk bumps the record cursor
 * before the counter, which is what leaves the counter's add after the two
 * halfword clears. These are source-shape facts about this function.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHalfword2EAndOriginZInvokeOriginHelperTickCountdownReload32ThenResetRecordTableScenarioSce15ef100_801D0DD8(void) {
  u8* work;
  u8* motion;
  u8* record;
  u16 advance;
  u8 count;
  s32 originX;
  s32 originY;
  s32 originZ;

  work = D_1F800044;
  *(u32*)(work + 0x6C) += 0xC0000;
  advance = *(u16*)(work + 0x2E) - 6;
  *(u16*)(work + 0x2E) = advance;
  func_801D147C(work + 0x64, advance, 6);
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 0x20;
    D_1F800044[6] = 0;
    D_1F800044[1]++;
    clearRecordActiveFlagsAt801D39A8ScenarioSce15ef100_801D199C();
    motion = D_1F800044;
    originX = *(s32*)(motion + 0x64);
    originY = *(s32*)(motion + 0x68);
    originZ = *(s32*)(motion + 0x6C);
    *(s32*)(motion + 0x34) = originX;
    *(s32*)(motion + 0x38) = originY;
    *(s32*)(motion + 0x3C) = originZ;
    record = D_801D3B28;
    count = 0u;
    do {
      func_801D1F68(record);
      count += 1u;
      record += 0x2Cu;
    } while (count < 0x10u);
    if (count < 0x20u) {
      do {
        *(u16*)(record + 0x28) = 0;
        *(u16*)(record + 0x2A) = 0;
        record += 0x2Cu;
        count += 1u;
      } while (count < 0x20u);
    }
  }
}
