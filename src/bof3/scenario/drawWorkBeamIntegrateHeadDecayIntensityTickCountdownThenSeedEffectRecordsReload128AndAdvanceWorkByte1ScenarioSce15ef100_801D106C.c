#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Table of 0x2C-byte effect records at 0x801D3DE8, after the 0x20-entry
 * 0x2C-byte-stride table the exact lift
 * advanceWorkHalfword2EAndOriginZInvokeOriginHelperTickCountdownReload32ThenResetRecordTableScenarioSce15ef100_801D0DD8
 * walks at 0x801D3B28 and inside the same Splat `textbin` row. seedEffectRecordFromWorkPositionWithRandomCornerRotationAndScaleScenarioSce15ef100_801D1F68
 * seeds one record from the scratchpad work object, so each walk of this table
 * re-stamps the records from the object's current points. */
extern u8 D_801D3DE8[]; /* @source 0x801D3DE8 @kind data */

void claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8(void);

/* Overlay work-record pass at 0x801D1A60: it ticks the life pair of every
 * claimed record of the 0x801D39A8 table and draws the claimed ones. Its result
 * is not consumed here. */
u32 stepClaimedWorkRecordsDecrementLifePairClearExpiredAndDrawAt801D39A8ScenarioSce15ef100_801D1A60(void);

void drawEffectRecordsAt801D3B28ThenAdvanceAngle24By10ScenarioSce15ef100_801D2088(void);
/* Overlay beam draw helper at 0x801D1678: draws the translucent quad pair
 * between the head point at +0x18 and the point at +0x34 of the work object and
 * then its stepped trail, shading both with the intensity byte it is handed. */
void drawBeamSliverPairBetweenHeadAndTrailPointThenSteppedTrailSegmentsWhileHeadXLagsStepXScenarioSce15ef100_801D1678(
    u8* from, u8* to, u8 intensity);
void seedEffectRecordFromWorkPositionWithRandomCornerRotationAndScaleScenarioSce15ef100_801D1F68(u8* record);
void swapWords(u32* arg0, u32* arg1);

/* @source 0x801D106C
 * @behavior Entry 6 of the overlay's step-handler code-pointer run at
 * 0x801D3918 (D_801D3918), which the overlay entry func_801D0C04 selects with
 * the unsigned work byte at offset +0x1 of the scratchpad work object published
 * at pointer slot 0x1F800044 and calls with no arguments and no result
 * consumed. It advances that object's frame byte at +0x6 by one and, when the
 * value read before the advance is a multiple of four, calls the record claim
 * at 0x801D19C8; it then calls 0x801D1A60 and 0x801D2088 in that order and
 * hands the object's head point at +0x18 and the point at +0x34 to the overlay
 * draw helper 0x801D1678 with intensity byte 0x40, which renders the
 * semi-transparent quad between those two points. It next integrates the object's
 * three-word step at +0x0C, +0x10 and +0x14 by adding the origin triple at
 * +0x64, +0x68 and +0x6C into it and then adding that step into the head point,
 * and re-seeds the head point with 0x2F8000, 0x278000 and 0x0F000000 whenever
 * the new first head word compares at or below 0x2F7FFF. It then subtracts 4
 * from the intensity byte at +0x3 and ticks the countdown byte at +0x9 down by
 * one; only when that countdown reaches zero does it exchange the head point
 * with the point at +0x34 word by word, call seedEffectRecordFromWorkPositionWithRandomCornerRotationAndScaleScenarioSce15ef100_801D1F68 on 0x10 consecutive
 * 0x2C-byte records of the effect table at 0x801D3DE8 counted from 0x10 up to
 * 0x20 in an unsigned byte, exchange those two points back, re-arm the
 * countdown with 0x80 and advance the work byte at +0x1 by one. It takes no
 * arguments, returns nothing and its 0x20-byte frame keeps $ra at 0x18, the
 * record cursor at 0x14 and the unsigned record counter at 0x10 across its
 * calls.
 *
 * Evidence: the run word at 0x801D3930 names 0x801D106C and the words
 * 0x801D391C, 0x801D3920, 0x801D3924, 0x801D3928, 0x801D392C, 0x801D3934 and
 * 0x801D3938 name 0x801D0CC0, 0x801D0D54, 0x801D0DD8, 0x801D0F10, 0x801D0FEC,
 * 0x801D12A0 and 0x801D13F4, so this body is element 6 of the nine-entry run
 * whose base 0x801D3918 the overlay entry loads after `sll $v0,$v0,2` of the
 * work byte at +0x1. The helper pair and the claim cadence are the calls the
 * exact siblings func_801D0F10 and func_801D0FEC make with the same
 * `lbu` / `addiu` / `sb` / `andi ...,0x3` / `bnez` group; the claim itself is
 * the record claim the exact lift
 * claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8 records.
 * drawBeamSliverPairBetweenHeadAndTrailPointThenSteppedTrailSegmentsWhileHeadXLagsStepXScenarioSce15ef100_801D1678
 * builds the POLY_G4 quad pair at g_PrimCursor with SetSemiTrans, projects
 * its two argument points through func_801AFF04 and widens each projected pair
 * by one screen unit, so the two triples handed to it at +0x18 and +0x34 are
 * the quad's two endpoints and its third argument is the quad's intensity byte.
 * seedEffectRecordFromWorkPositionWithRandomCornerRotationAndScaleScenarioSce15ef100_801D1F68 copies the object's words at +0x34, +0x38 and +0x3C into the
 * record it is given and fills that record's angle fields from rand, so during
 * the walk the exchange makes the table records read the head point.
 *
 * Shape evidence: the head group, the scratchpad cell spelling, the
 * origin/step integration with three re-read fields and the re-arm tail are the
 * statement order of the exact sibling func_801D0F10 written with this body's
 * own offsets and constants; the three-way word exchange is the exact lift
 * swapWords' caller shape; the table walk is the do/while pointer-and-unsigned
 * byte counter shape of the exact lifts func_801D0DD8 and
 * clearRecordActiveFlagsAt801D39A8ScenarioSce15ef100_801D199C, including the
 * `andi ...,0xFF` a `u8` counter's bound test emits.
 *
 * Shape decisions measured on this body: the frame tick and the draw group each
 * write the scratchpad cell into their own pointer local, which is what keeps
 * the head pointer in $v0 and the draw-and-exchange pointer in $a1; one local
 * over both measured 110/141 with the first difference at instruction 0, where
 * the head load took $a1 and the frame byte moved to $v0. The draw call and all six
 * word exchanges share that second local, so the exchanges before the table walk
 * and after it both take $a1; giving the exchanges after the walk the
 * integration local measured 129/141 with the first difference at instruction
 * 106, the first exchange of that second group. The integration chain keeps a
 * third local of its own, which the original holds in $a3 from its first load to
 * the reset stores. The reset block re-reads the head word it compares instead
 * of reusing the value it just stored, and every inline countdown or byte store
 * re-reads the cell, because a store or a call between two uses may change the
 * published pointer. These are source-shape facts about this function, not a
 * general rule.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkBeamIntegrateHeadDecayIntensityTickCountdownThenSeedEffectRecordsReload128AndAdvanceWorkByte1ScenarioSce15ef100_801D106C(void) {
  u8* work;
  u8* point;
  u8* motion;
  u8* record;
  u8 frame;
  u8 count;

  work = D_1F800044;
  frame = work[6];
  work[6] = frame + 1;
  if ((frame & 3) == 0) {
    claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8();
  }
  stepClaimedWorkRecordsDecrementLifePairClearExpiredAndDrawAt801D39A8ScenarioSce15ef100_801D1A60();
  drawEffectRecordsAt801D3B28ThenAdvanceAngle24By10ScenarioSce15ef100_801D2088();
  point = D_1F800044;
  drawBeamSliverPairBetweenHeadAndTrailPointThenSteppedTrailSegmentsWhileHeadXLagsStepXScenarioSce15ef100_801D1678(
      point + 0x18, point + 0x34, 0x40);
  motion = D_1F800044;
  *(s32*)(motion + 0x0C) += *(s32*)(motion + 0x64);
  *(s32*)(motion + 0x10) += *(s32*)(motion + 0x68);
  *(s32*)(motion + 0x14) += *(s32*)(motion + 0x6C);
  *(s32*)(motion + 0x18) += *(s32*)(motion + 0x0C);
  *(s32*)(motion + 0x1C) += *(s32*)(motion + 0x10);
  *(s32*)(motion + 0x20) += *(s32*)(motion + 0x14);
  if (*(s32*)(motion + 0x18) <= 0x2F7FFF) {
    *(s32*)(motion + 0x18) = 0x2F8000;
    *(s32*)(motion + 0x1C) = 0x278000;
    *(s32*)(motion + 0x20) = 0xF000000;
  }
  D_1F800044[3] -= 4;
  if (--D_1F800044[9] == 0) {
    point = D_1F800044;
    swapWords((u32*)(point + 0x34), (u32*)(point + 0x18));
    point = D_1F800044;
    swapWords((u32*)(point + 0x38), (u32*)(point + 0x1C));
    point = D_1F800044;
    swapWords((u32*)(point + 0x3C), (u32*)(point + 0x20));
    record = D_801D3DE8;
    count = 0x10;
    do {
      seedEffectRecordFromWorkPositionWithRandomCornerRotationAndScaleScenarioSce15ef100_801D1F68(record);
      count += 1u;
      record += 0x2Cu;
    } while (count < 0x20u);
    point = D_1F800044;
    swapWords((u32*)(point + 0x34), (u32*)(point + 0x18));
    point = D_1F800044;
    swapWords((u32*)(point + 0x38), (u32*)(point + 0x1C));
    point = D_1F800044;
    swapWords((u32*)(point + 0x3C), (u32*)(point + 0x20));
    D_1F800044[9] = 0x80;
    D_1F800044[1] += 1;
  }
}
