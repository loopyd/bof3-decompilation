#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared scenario progress byte at 0x80146864. The sibling entry 4 of this run
 * reads it as an unsigned byte and compares it with 0x14, so it keeps the
 * shared-map spelling and byte width. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; it consumes one cue id and returns
 * nothing.
 */
extern void dispatchSoundCue(u32 cue_id);

void claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8(void);
void func_801D1A60(void);
void func_801D2088(void);
void func_801D1678(u8* from, u8* to, u8 intensity);
void swapWords(u32* arg0, u32* arg1);

/* @source 0x801D12A0
 * @behavior Entry 7 of the overlay's step-handler code-pointer run at
 * 0x801D3918 (D_801D3918), which the overlay entry func_801D0C04 selects with
 * the unsigned work byte at offset +0x1 of the scratchpad work object published
 * at pointer slot 0x1F800044 and calls with no arguments and no result
 * consumed. It advances that object's frame byte at +0x6 by one and, when the
 * value read before the advance is a multiple of four, calls the record claim
 * at 0x801D19C8, exchanges the object's head triple at +0x18, +0x1C and +0x20
 * with the point triple at +0x34, +0x38 and +0x3C word by word, calls the same
 * claim a second time and exchanges the two triples back. It then calls
 * 0x801D1A60 and 0x801D2088 in that order and hands the head triple at +0x18
 * and the point triple at +0x34 to the overlay draw helper 0x801D1678 with the
 * constant intensity 0x40, which renders the semi-transparent quad between
 * those two points. Finally, only when the shared scenario progress byte
 * 0x80146864 reads 0x17, it sets the object's intensity byte at +0x3 to 0x80,
 * arms the countdown byte at +0x9 with 8, advances the work byte at +0x1 by one
 * so the next dispatch selects entry 8 (0x801D13F4), and dispatches sound cue
 * 0x20A through the resident executable cue dispatcher at 0x8015DF18. The
 * scratchpad cell is re-read before every store and before every store pair,
 * because a call or a store through the loaded pointer may republish it. The
 * 0x18-byte frame exists only to keep $ra at 0x10 across the calls; nothing is
 * spilled.
 *
 * Name evidence: the only in-image reference to 0x801D12A0 is the word at
 * 0x801D3934, which is element 7 of the nine-word code-pointer run based at
 * D_801D3918 (read from the original bytes: 0x801D0C48, 0x801D0CC0, 0x801D0D54,
 * 0x801D0DD8, 0x801D0F10, 0x801D0FEC, 0x801D106C, 0x801D12A0, 0x801D13F4), so
 * this body is entry 7 of that run and no other site names it. The claim,
 * exchange, draw and progress-gated arming roles come from this body's own
 * calls, offsets and literals: +0x6 is the frame byte the exact sibling entries
 * 5 and 6 advance with the same modulo-four claim cadence, +0x1, +0x3 and +0x9
 * are the work, intensity and countdown bytes the exact siblings
 * drawWorkBeamIntegrateHeadDecayIntensityTickCountdownThenSeedEffectRecordsReload128AndAdvanceWorkByte1ScenarioSce15ef100_801D106C
 * and
 * decayWorkHalfword2EAndIntensityByte3InvokeHelperDrawBeamThenResetWorkStateWhenCountdown9ExpiresScenarioSce15ef100_801D13F4
 * read and write, the +0x18/+0x34 point pair is the pair those siblings draw
 * between with intensity 0x40, 0x80146864 is the shared scenario progress byte
 * that the exact sibling entry 4 compares with 0x14, and 0x20A is one cue id
 * above the 0x209 the exact sibling entry 5 dispatches.
 *
 * Shape evidence: the two claims with the three-word exchange between and after
 * them, and the per-cell `x = D_1F800044; x[...] = ...` reload spelling on both
 * sides, are the exact shapes of
 * drawWorkBeamIntegrateHeadDecayIntensityTickCountdownThenSeedEffectRecordsReload128AndAdvanceWorkByte1ScenarioSce15ef100_801D106C
 * in this overlay; this body differs by exchanging unconditionally on the claim
 * cadence instead of on countdown expiry, by drawing before the progress test
 * rather than after integration, and by re-arming intensity, countdown and work
 * byte behind `g_ScenarioProgress == 0x17` instead of decaying them.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimRecordExchangeHeadStepPairTwiceDrawBeamThenOnProgress23ArmIntensity128Countdown8DispatchCue522AndAdvanceWorkByte1ScenarioSce15ef100_801D12A0(void) {
  u8* work;
  u8* point;
  u8* intensity;
  u8* countdown;
  u8* advance;
  u8 frame;

  work = D_1F800044;
  frame = work[6];
  work[6] = frame + 1;
  if ((frame & 3) == 0) {
    claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8();
    point = D_1F800044;
    swapWords((u32*)(point + 0x34), (u32*)(point + 0x18));
    point = D_1F800044;
    swapWords((u32*)(point + 0x38), (u32*)(point + 0x1C));
    point = D_1F800044;
    swapWords((u32*)(point + 0x3C), (u32*)(point + 0x20));
    claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8();
    point = D_1F800044;
    swapWords((u32*)(point + 0x34), (u32*)(point + 0x18));
    point = D_1F800044;
    swapWords((u32*)(point + 0x38), (u32*)(point + 0x1C));
    point = D_1F800044;
    swapWords((u32*)(point + 0x3C), (u32*)(point + 0x20));
  }
  func_801D1A60();
  func_801D2088();
  point = D_1F800044;
  func_801D1678(point + 0x18, point + 0x34, 0x40);
  if (g_ScenarioProgress == 0x17) {
    intensity = D_1F800044;
    intensity[3] = 0x80;
    countdown = D_1F800044;
    countdown[9] = 8;
    advance = D_1F800044;
    advance[1]++;
    dispatchSoundCue(0x20A);
  }
}
