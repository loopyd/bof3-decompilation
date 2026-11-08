#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Overlay draw helper at 0x801D36BC: takes the point record at its first
 * argument, whose first word is a vertex table base stepped in 0x10-byte rows,
 * uses the second argument as that row index and as one scale factor and the
 * third as a second scale factor, projects the record through 0x801AFF04 and
 * emits one semi-transparent quad through SetPolyG4. Its role beyond that is
 * not established here, so it stays address-anchored. */
void func_801D36BC(s32* point, s32 row, s32 scale);

/* Resident executable reset helper at 0x80196070, which zeroes work-object
 * bytes 0x00-0x04. */
void func_80196070(void);

/* @source 0x801D2D5C
 * @behavior Entry 9 of the overlay's step-handler code-pointer run at
 * 0x801D3954, which the dispatcher at 0x801D2170 selects with the unsigned work
 * byte at offset +0x1 of the scratchpad work object published at pointer slot
 * 0x1F800044 and calls with no arguments and no result consumed. Through one
 * load of that cell it hands the object's point at +0x34 and the object's word
 * pair at +0x0C/+0x10 to the overlay draw helper 0x801D36BC; it then re-reads
 * the cell and, through that second pointer, adds the head point pair at
 * +0x18/+0x1C into the word pair at +0x0C/+0x10, subtracts 0x100 from +0x18 and
 * 0x200 from +0x1C, and ticks the countdown byte at +0x9 down by one through
 * that same pointer, calling the resident reset helper 0x80196070 when the
 * decremented countdown reads zero. The scratchpad cell is re-read after the
 * call because the callee may republish it. The 0x18-byte frame exists only to
 * keep $ra at 0x10 across the two calls.
 *
 * Name evidence: the only in-image reference to 0x801D2D5C is the word at
 * 0x801D3978, the last of the ten words of the code-pointer run based at
 * 0x801D3954 (0x801D21B4, 0x801D23A0, 0x801D2474, 0x801D26BC, 0x801D2934,
 * 0x801D2B44, 0x801D2BA8, 0x801D2C00, 0x801D2C94, 0x801D2D5C) that the
 * dispatcher 0x801D2170 indexes with work byte +0x1 shifted left by two, so
 * this body is entry 9 of that run and no other site names it. The pair roles
 * come from the offsets the exact lifts of this overlay already own:
 * +0x0C/+0x10 are the first two words of the step triple and +0x18/+0x1C the
 * first two of the head point, as
 * clearStepAndHeadXYWordsArmCountdown20DispatchCue208AndAdvanceWorkByte1ScenarioSce15ef100_801D2BA8
 * clears them and
 * drawWorkBeamIntegrateHeadDecayIntensityTickCountdownThenSeedEffectRecordsReload128AndAdvanceWorkByte1ScenarioSce15ef100_801D106C
 * integrates them; +0x34 is the point those siblings draw from, +0x9 the
 * countdown byte they tick, and 0x80196070 the resident reset helper they call
 * when it expires.
 *
 * Shape evidence: the drawn point plus the cell re-read that feeds the whole
 * word run is the statement order of the exact sibling
 * decayWorkHalfword2EAndIntensityByte3InvokeHelperDrawBeamThenResetWorkStateWhenCountdown9ExpiresScenarioSce15ef100_801D13F4,
 * the `*(s32*)(ptr + ..) += *(s32*)(ptr + ..)` integration pairing is the
 * spelling of the exact sibling func_801D106C, and the `--state[9] == 0` tail is
 * that countdown test written through the group's own pointer local. One local
 * shared by the pre-call group and the word run measured 11/36 with the first
 * difference in the cell load's register, because the run's stores then block
 * the four integration loads from scheduling ahead of the decrement's store; the
 * separate pointer local for the run is what keeps them ahead of it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkQuadIntegrateHeadPairIntoStepPairDecayHeadPairTickCountdownThenResetWorkStateScenarioSce15ef100_801D2D5C(void) {
  u8* work;
  u8* state;

  work = D_1F800044;
  func_801D36BC((s32*)(work + 0x34), *(s32*)(work + 0x0C), *(s32*)(work + 0x10));
  state = D_1F800044;
  *(s32*)(state + 0x0C) += *(s32*)(state + 0x18);
  *(s32*)(state + 0x10) += *(s32*)(state + 0x1C);
  *(s32*)(state + 0x18) -= 0x100;
  *(s32*)(state + 0x1C) -= 0x200;
  if (--state[9] == 0) {
    func_80196070();
  }
}
