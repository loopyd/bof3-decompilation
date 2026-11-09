#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; it consumes one cue id and returns
 * nothing.
 */
extern void dispatchSoundCue(u32 cue_id);

void claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8(void);

/* Overlay work-record pass at 0x801D1A60: it ticks the life pair of every
 * claimed record of the 0x801D39A8 table and draws the claimed ones. Its result
 * is not consumed here. */
u32 stepClaimedWorkRecordsDecrementLifePairClearExpiredAndDrawAt801D39A8ScenarioSce15ef100_801D1A60(void);

void drawEffectRecordsAt801D3B28ThenAdvanceAngle24By10ScenarioSce15ef100_801D2088(void);

/* @source 0x801D0FEC
 * @behavior Entry 5 of the overlay's step-handler code-pointer run at
 * 0x801D3918 (D_801D3918), which the overlay entry func_801D0C04 selects with
 * the unsigned work byte at offset +0x1 of the scratchpad work object published
 * at pointer slot 0x1F800044 and calls with no arguments and no result
 * consumed. It advances that object's frame byte at +0x6 by one and, when the
 * value read before the advance is a multiple of four, calls the record claim
 * at 0x801D19C8. It then calls 0x801D2088 and 0x801D1A60 in that order, arms
 * the object's countdown byte at +0x9 with 0x20, advances the work byte at +0x1
 * by one so the next dispatch selects entry 6 (0x801D106C), and dispatches
 * sound cue 0x209 through the resident executable cue dispatcher at 0x8015DF18.
 * The scratchpad cell is re-read once per store group because a store through
 * the loaded pointer may alias that cell: the frame tick and the claim cadence
 * run through the first load in $v0, the countdown arm through the second load
 * in $v1, and the state-byte advance through the third load in $v1. Nothing
 * stays live across the cue call - the advanced byte is committed before it -
 * so the whole body runs in $v0/$v1/$a0 and the 0x18-byte frame exists only to
 * keep $ra at 0x10 across the three calls.
 *
 * Name evidence: the only in-image reference to 0x801D0FEC is word 5 of the
 * ten-word run at 0x801D3918, which the exact dispatcher
 * dispatchWorkByte1HandlerScenarioSce15ef100_801D2170 indexes with work byte
 * +0x1; there is no jal caller, which is the dispatch shape rather than missing
 * evidence. The advance-claim-arm-cue roles come from this body's own literals
 * and offsets and from the field roles the already-reviewed entries of the same
 * run document.
 *
 * Shape evidence: the first ten instructions of this body are the exact lift
 * advanceWorkByte6ClaimRecordInvokeTwoHelpersAndOnProgress20RestoreMotionStateAndAdvanceWorkByte1ScenarioSce15ef100_801D0F10
 * (0x801D0F10, entry 4 of the same run) byte for byte, so the scratchpad cell
 * spelling, the frame-tick local and the claim cadence are copied from it. The
 * countdown arm through a fresh cell load and the sound-cue dispatch that
 * follows the state-byte increment are the pairing the exact sibling
 * clearStepAndHeadXYWordsArmCountdown20DispatchCue208AndAdvanceWorkByte1ScenarioSce15ef100_801D2BA8
 * (0x801D2BA8, entry 6 of the ten-word run at 0x801D3954) makes with cue 0x208
 * and countdown 0x14. This function's own offsets, countdown value and cue id
 * come from its original bytes: an increment carried in its own local and
 * stored after the cue call made the value live across that call, which cost
 * two callee-saved registers and a 0x20-byte frame (22 of 32 instructions,
 * 128 -> 144 bytes), while the in-place `advance[1]++` before the call leaves
 * the cue-argument load filling the load-delay gap and the store filling the
 * call's delay slot, which is the measured 32 of 32 instructions and 128 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte6ClaimRecordInvokeTwoHelpersArmCountdown32DispatchCue209AndAdvanceWorkByte1ScenarioSce15ef100_801D0FEC(void) {
  u8* work;
  u8* countdown;
  u8* advance;
  u8 frame;

  work = D_1F800044;
  frame = work[6];
  work[6] = frame + 1;
  if ((frame & 3) == 0) {
    claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8();
  }
  drawEffectRecordsAt801D3B28ThenAdvanceAngle24By10ScenarioSce15ef100_801D2088();
  stepClaimedWorkRecordsDecrementLifePairClearExpiredAndDrawAt801D39A8ScenarioSce15ef100_801D1A60();
  countdown = D_1F800044;
  countdown[9] = 0x20;
  advance = D_1F800044;
  advance[1]++;
  dispatchSoundCue(0x209);
}
