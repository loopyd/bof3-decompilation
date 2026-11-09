#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared scenario progress byte, bound by the composed shared map at
 * 0x80146864; this overlay reads it as an unsigned byte. */
extern u8 g_ScenarioProgress;

void claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8(void);

/* Overlay work-record pass at 0x801D1A60: it ticks the life pair of every
 * claimed record of the 0x801D39A8 table and draws the claimed ones. Its result
 * is not consumed here. */
u32 stepClaimedWorkRecordsDecrementLifePairClearExpiredAndDrawAt801D39A8ScenarioSce15ef100_801D1A60(void);

void drawEffectRecordsAt801D3B28ThenAdvanceAngle24By10ScenarioSce15ef100_801D2088(void);

/* @source 0x801D0F10
 * @behavior Entry 4 of the overlay's step-handler code-pointer run at
 * 0x801D3918 (D_801D3918), which the overlay entry func_801D0C04 selects with
 * the unsigned work byte at offset +0x1 of the scratchpad work object published
 * at pointer slot 0x1F800044 and calls with no arguments and no result
 * consumed. It advances that object's frame byte at +0x6 by one and, when the
 * value read before the advance is a multiple of four, calls the record claim
 * at 0x801D19C8. It then calls 0x801D2088 and 0x801D1A60 in that order. When
 * the shared scenario progress byte reads 0x14 it reloads the object once and
 * rewrites its motion state: the three words at +0x64, +0x68 and +0x6C are
 * copied into the position triple at +0x18, +0x1C and +0x20, the step triple at
 * +0x0C, +0x10 and +0x14 is cleared to zero, each of +0x64, +0x68 and +0x6C is
 * re-armed with the value (0x2F8000 - v) >> 7, (0x278000 - v) >> 7 and
 * (0x0F000000 - v) >> 7 read from the field it replaces, and the work byte at
 * +0x1 is advanced by one so the next dispatch leaves this entry. Every field
 * access after the two helper calls goes through the one reloaded object
 * pointer, the three re-arm reads are separate loads of the fields they
 * overwrite, and the advanced byte at +0x1 is carried in its own local from the
 * read beside the origin triple to a store that lands after all three re-arms.
 * It takes no arguments, returns nothing and its 0x18-byte frame exists only to
 * keep $ra at 0x10 across the three calls.
 *
 * Evidence: the run word at 0x801D3928 names 0x801D0F10, and the words
 * 0x801D391C, 0x801D3920, 0x801D3924, 0x801D392C and 0x801D3930 name
 * 0x801D0CC0, 0x801D0D54, 0x801D0DD8, 0x801D0FEC and 0x801D106C, so this body
 * is element 4 of the run whose base 0x801D3918 the overlay entry loads after
 * `sll $v0,$v0,2` of the work byte at +0x1. The `lbu` / `addiu` / `sb` /
 * `andi ...,0x3` / `bnez` group at 0x801D0F20 is the claim cadence this
 * target's claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8
 * records for entries 4, 5 and 6 of the same run. The two helper calls are
 * 0x801D2088, which primes a tpage and draw mode and walks the 0x20 entries of
 * the 0x2C-stride table at 0x801D3B28, and 0x801D1A60, which walks the 0x10
 * entries of the 0x18-stride record table at 0x801D39A8 and ticks each claimed
 * entry.
 *
 * Field roles: the words at +0x18, +0x1C and +0x20 are the position triple the
 * next span func_801D13F4 advances by the triple at +0x0C, +0x10 and +0x14 and
 * resets to exactly 0x2F8000, 0x278000 and 0x0F000000 when its first word rises
 * above 0x2F7FFF, so the three re-arm constants written here are that same
 * reset triple reflected around the current value. The exact lift
 * advanceWorkHalfword2EThenInvokeOriginHelperTickCountdownReload64AndAdvanceWorkByte1ScenarioSce15ef100_801D0CC0
 * hands +0x64 to the overlay helper 0x801D147C as the object's origin block,
 * and func_801D13F4 swaps +0x18, +0x1C and +0x20 with +0x34, +0x38 and +0x3C
 * around its second claim call, which is what pins the copy direction here to
 * origin-into-position.
 *
 * Shape evidence: the same file's entry 1 lift fixes the spelling of the
 * scratchpad cell (a named `D_1F800044` extern folds to `lui 0x1F80` plus
 * `lw 0x44(reg)`, while the shared accessor macro would emit `lui`/`ori` plus a
 * displacement-0 load) and the `extern u8 g_ScenarioProgress` declaration this
 * body needs for its `lbu`. The scratchpad pointer is reloaded after the helper
 * calls and then reused for every field access in the guarded block, so one
 * local carries the whole tail, and the three re-arm expressions each re-read
 * the field they overwrite because a store through the same byte pointer may
 * alias it.
 *
 * Shape decisions measured on this body: the frame tick and the guarded block
 * address the cell through two separate pointer locals, because one shared local
 * made the single quantity take one register across the whole function and lost
 * the original's `$v0` for the tick and `$v1` for the tail. The advanced byte is
 * likewise read into its own local and committed by a store that follows the
 * re-arms, which is what keeps the load-advance pair ahead of the store group;
 * storing `motion[1] + 1` in place put that pair inside the re-arm chain and
 * left 9 of the 55 instructions out of order. Both are source-shape facts about
 * this function, not a general rule.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte6ClaimRecordInvokeTwoHelpersAndOnProgress20RestoreMotionStateAndAdvanceWorkByte1ScenarioSce15ef100_801D0F10(void) {
  u8* work;
  u8* motion;
  u8 frame;
  u8 next;
  s32 originX;
  s32 originY;
  s32 originZ;

  work = D_1F800044;
  frame = work[6];
  work[6] = frame + 1;
  if ((frame & 3) == 0) {
    claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8();
  }
  drawEffectRecordsAt801D3B28ThenAdvanceAngle24By10ScenarioSce15ef100_801D2088();
  stepClaimedWorkRecordsDecrementLifePairClearExpiredAndDrawAt801D39A8ScenarioSce15ef100_801D1A60();
  if (g_ScenarioProgress == 0x14) {
    motion = D_1F800044;
    originX = *(s32*)(motion + 0x64);
    originY = *(s32*)(motion + 0x68);
    originZ = *(s32*)(motion + 0x6C);
    next = motion[1] + 1;
    *(s32*)(motion + 0x0C) = 0;
    *(s32*)(motion + 0x10) = 0;
    *(s32*)(motion + 0x14) = 0;
    *(s32*)(motion + 0x18) = originX;
    *(s32*)(motion + 0x1C) = originY;
    *(s32*)(motion + 0x20) = originZ;
    *(s32*)(motion + 0x64) = (0x2F8000 - *(s32*)(motion + 0x64)) >> 7;
    *(s32*)(motion + 0x68) = (0x278000 - *(s32*)(motion + 0x68)) >> 7;
    *(s32*)(motion + 0x6C) = (0x0F000000 - *(s32*)(motion + 0x6C)) >> 7;
    motion[1] = next;
  }
}
