#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Overlay work-record pass at 0x801D1A60, one of the two helpers every step
 * handler of this overlay's run at 0x801D3918 calls before it draws: it ticks
 * the life pair of every claimed record of the 0x801D39A8 table and draws the
 * claimed ones. Its result is not consumed here. */
u32 stepClaimedWorkRecordsDecrementLifePairClearExpiredAndDrawAt801D39A8ScenarioSce15ef100_801D1A60(void);

/* Overlay beam draw helper at 0x801D1678: draws the translucent quad pair
 * between the head point at +0x18 and the point at +0x34 of the work object and
 * then its stepped trail, shading both with the intensity byte it is handed. */
void drawBeamSliverPairBetweenHeadAndTrailPointThenSteppedTrailSegmentsWhileHeadXLagsStepXScenarioSce15ef100_801D1678(
    u8* from, u8* to, u8 intensity);

/* Resident executable reset helper at 0x80196070, which zeroes work-object
 * bytes 0x00-0x04. */
void func_80196070(void);

/* @source 0x801D13F4
 * @behavior Entry 8 of the overlay's step-handler code-pointer run at
 * 0x801D3918, which the overlay entry func_801D0C04 selects with the unsigned
 * work byte at offset +0x1 of the scratchpad work object published at pointer
 * slot 0x1F800044 and calls with no arguments and no result consumed. Through
 * one load of that cell it subtracts 0x10 from the unsigned work halfword at
 * +0x2E and 8 from the intensity byte at +0x3, storing both back through the
 * same pointer; it then calls the overlay helper 0x801D1A60, re-reads the cell
 * and hands the object's head point at +0x18 and the point at +0x34 to the
 * overlay draw helper together with the just-decayed intensity byte
 * at +0x3. It finally ticks the object's countdown byte at +0x9 down by one
 * through a third load of the cell and, when that countdown reaches zero, calls
 * the resident reset helper 0x80196070. The scratchpad cell is re-read after
 * each call because the callee may republish it. The 0x18-byte frame exists
 * only to keep $ra at 0x10 across the three calls.
 *
 * Name evidence: the only in-image reference to 0x801D13F4 is the word at
 * 0x801D3938, which is word 8 of the nine-word code-pointer run based at
 * D_801D3918 (the overlay entry func_801D0C04 shifts work byte +0x1 left by two
 * and indexes that base), so this body is entry 8 of the run and no other site
 * names it. The decay, draw and reset roles come from this body's own literals
 * and offsets: +0x2E is the unsigned work halfword the exact siblings
 * advanceWorkHalfword2EThenInvokeOriginHelperTickCountdownReload64AndAdvanceWorkByte1ScenarioSce15ef100_801D0CC0
 * and
 * advanceWorkHalfword2EAndOriginZInvokeOriginHelperTickCountdownReload32ThenResetRecordTableScenarioSce15ef100_801D0DD8
 * advance, +0x3 is the intensity byte the exact sibling
 * drawWorkBeamIntegrateHeadDecayIntensityTickCountdownThenSeedEffectRecordsReload128AndAdvanceWorkByte1ScenarioSce15ef100_801D106C
 * decays by 4 and +0x9 its countdown byte, the +0x18/+0x34 point pair is the
 * pair that sibling draws between, and 0x80196070 is the resident reset helper
 * the exact lifts in the neighbouring scenario overlays call to clear work
 * bytes 0x00-0x04.
 *
 * Shape evidence: the one-cell-load spelling of the two decrements, the
 * post-call cell re-read that feeds the draw call, and the
 * `if (--D_1F800044[9] == 0)` countdown tail are the exact shapes of
 * drawWorkBeamIntegrateHeadDecayIntensityTickCountdownThenSeedEffectRecordsReload128AndAdvanceWorkByte1ScenarioSce15ef100_801D106C
 * in this overlay; this body differs by passing the decayed +0x3 byte instead
 * of the constant 0x40 and by calling the reset helper instead of re-arming.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decayWorkHalfword2EAndIntensityByte3InvokeHelperDrawBeamThenResetWorkStateWhenCountdown9ExpiresScenarioSce15ef100_801D13F4(void) {
  u8* work;
  u8* point;

  work = D_1F800044;
  *(u16*)(work + 0x2E) -= 0x10;
  work[3] -= 8;
  stepClaimedWorkRecordsDecrementLifePairClearExpiredAndDrawAt801D39A8ScenarioSce15ef100_801D1A60();
  point = D_1F800044;
  drawBeamSliverPairBetweenHeadAndTrailPointThenSteppedTrailSegmentsWhileHeadXLagsStepXScenarioSce15ef100_801D1678(
      point + 0x18, point + 0x34, point[3]);
  if (--D_1F800044[9] == 0) {
    func_80196070();
  }
}
