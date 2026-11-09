#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Overlay quad ring emitter at 0x801D36BC, lifted at that address: it takes the
 * three-word point record at its first argument and the two ring radii in the
 * second and third, projects every corner formed by rotating one radius in the
 * x/y plane of that point, and emits thirty-two semi-transparent quads through
 * SetPolyG4. */
void drawTranslucentGouraudRingOfThirtyTwoQuadsAroundProjectedPointBetweenTwoRadiiFadingFromOrangeToBlackScenarioSce15ef100_801D36BC(
    s32* point, s32 orangeRadius, s32 blackRadius);

/* Resident executable reset helper at 0x80196070, which zeroes work-object
 * bytes 0x00-0x04. */
void func_80196070(void);

/* @source 0x801D2C00
 * @behavior Entry 7 of the overlay's step-handler code-pointer run at
 * 0x801D3954, which the dispatcher at 0x801D2170 selects with the unsigned work
 * byte at offset +0x1 of the scratchpad work object published at pointer slot
 * 0x1F800044 and calls with no arguments and no result consumed. Through one
 * load of that cell it moves the object's head point, adding 0x200 to the word
 * at +0x18 and 0x100 to the word at +0x1C, adds that head pair into the step
 * word pair at +0x0C/+0x10, and hands the object's point at +0x34 together with
 * the updated step pair to the overlay quad ring emitter 0x801D36BC as its two
 * radii. It then
 * re-reads the cell and, through that second pointer, ticks the countdown byte
 * at +0x9 down by one, calling the resident reset helper 0x80196070 when the
 * decremented countdown reads zero. The cell is re-read after the call because
 * the callee may republish it. The 0x18-byte frame exists only to keep $ra at
 * 0x10 across the two calls, and the third argument store lands in the draw
 * call's delay slot.
 *
 * Name evidence: the only in-image reference to 0x801D2C00 is the word at
 * 0x801D3970, word 7 of the ten words of the code-pointer run based at
 * 0x801D3954 (0x801D21B4, 0x801D23A0, 0x801D2474, 0x801D26BC, 0x801D2934,
 * 0x801D2B44, 0x801D2BA8, 0x801D2C00, 0x801D2C94, 0x801D2D5C) that the
 * dispatcher 0x801D2170 indexes with work byte +0x1 shifted left by two, so
 * this body is that run's entry 7 and no other site names it. The preceding
 * entry 6, clearStepAndHeadXYWordsArmCountdown20DispatchCue208AndAdvanceWorkByte1ScenarioSce15ef100_801D2BA8,
 * clears the same +0x0C/+0x10 and +0x18/+0x1C pairs before advancing the work
 * byte to select this body, which is the step-then-head order the name follows.
 * The pair roles come from the offsets the exact lifts of this overlay already
 * own: +0x0C/+0x10 are the first two words of the step triple and +0x18/+0x1C
 * the first two of the head point, +0x34 the point the siblings draw from,
 * +0x9 the countdown byte they tick, and 0x80196070 the resident reset helper
 * they call when it expires.
 *
 * Shape evidence: this body is the exact sibling
 * drawWorkQuadIntegrateHeadPairIntoStepPairDecayHeadPairTickCountdownThenResetWorkStateScenarioSce15ef100_801D2D5C
 * (entry 9 of the same run) with two statement reorderings and the opposite
 * head-point delta. That sibling establishes the pointer-cell spelling, the
 * `*(s32*)(ptr + a) += *(s32*)(ptr + b)` integration pairing, the draw through
 * 0x801D36BC from the +0x34 point with the step pair as the two ring radii,
 * the second cell load into a separate `state` local, and the `--state[9] == 0`
 * tail that calls 0x80196070. Here the head-point advance comes first and adds
 * its constants rather than subtracting, so it also differs in register: the
 * 0x1C word is advanced in $a0, which is why the 0x200 sits on +0x18 and the
 * 0x100 on +0x1C. The second cell load into its own local is the sibling
 * family's documented shape: the exact 0x801D2D5C lift records that one local
 * shared by its pre-call group and its word run reorders those loads, so this
 * body keeps the post-call countdown update behind a separate pointer local.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHeadPairIntegrateStepPairDrawQuadTickCountdownThenResetWorkStateScenarioSce15ef100_801D2C00(void) {
  u8* work;
  u8* state;

  work = D_1F800044;
  *(s32*)(work + 0x18) += 0x200;
  *(s32*)(work + 0x1C) += 0x100;
  *(s32*)(work + 0x0C) += *(s32*)(work + 0x18);
  *(s32*)(work + 0x10) += *(s32*)(work + 0x1C);
  drawTranslucentGouraudRingOfThirtyTwoQuadsAroundProjectedPointBetweenTwoRadiiFadingFromOrangeToBlackScenarioSce15ef100_801D36BC(
      (s32*)(work + 0x34), *(s32*)(work + 0x0C), *(s32*)(work + 0x10));
  state = D_1F800044;
  if (--state[9] == 0) {
    func_80196070();
  }
}
