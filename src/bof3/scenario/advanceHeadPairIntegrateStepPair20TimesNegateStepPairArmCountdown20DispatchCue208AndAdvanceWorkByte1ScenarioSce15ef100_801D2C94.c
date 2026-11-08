#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; it consumes one cue id and returns
 * nothing.
 */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801D2C94
 * @behavior Entry 8 of the overlay's step-handler code-pointer run at
 * 0x801D3954, which the dispatcher at 0x801D2170 selects with the unsigned work
 * byte at offset +0x1 of the scratchpad work object published at pointer slot
 * 0x1F800044 and calls with no arguments and no result consumed. Through one
 * load of that cell it clears the word pair at +0x0C/+0x10 - the first two words
 * of the step triple the reviewed siblings of this run advance - and the word
 * pair at +0x18/+0x1C, the first two words of the head point. It then runs a
 * twenty-iteration integration over the same pointer: each pass adds 0x100 to
 * the head word at +0x18 and 0x200 to the head word at +0x1C and folds the
 * updated head pair into the step pair at +0x0C/+0x10, so the head point walks
 * 0x1400 and 0x2800 in total while the step pair accumulates the whole path. It
 * re-reads the cell and, through that second pointer, arms the countdown byte at
 * +0x9 with 0x14 and negates the accumulated step pair word by word, then
 * re-reads the cell a third time and, through that pointer, advances the work
 * byte at +0x1 by one and dispatches sound cue 0x208 through the resident
 * executable cue dispatcher 0x8015DF18. The three cell loads are the original's
 * own: the integration stores through the first pointer, the arm-and-negate
 * group re-reads the cell because those stores may alias it, and the trailing
 * byte advance re-reads it again behind the arm group's byte store. Takes no
 * arguments and returns nothing; its 0x18-byte frame exists only to keep $ra at
 * 0x10 across the cue call, whose delay slot holds the incremented work byte.
 *
 * Name evidence: the only in-image reference to 0x801D2C94 is the word at
 * 0x801D3974, the ninth of the ten words of the code-pointer run based at
 * 0x801D3954 (0x801D21B4, 0x801D23A0, 0x801D2474, 0x801D26BC, 0x801D2934,
 * 0x801D2B44, 0x801D2BA8, 0x801D2C00, 0x801D2C94, 0x801D2D5C) that the
 * dispatcher 0x801D2170 indexes with work byte +0x1 shifted left by two, so this
 * body is entry 8 of that run and no other site names it. The pair roles come
 * from the offsets the exact lifts of this overlay already own: +0x0C/+0x10 are
 * the first two words of the step triple, +0x18/+0x1C the first two of the head
 * point, +0x9 the countdown byte and +0x1 the work byte the dispatcher reads.
 * The integration count, the 0x100/0x200 head deltas, the 0x14 countdown value
 * and cue 0x208 come from this body's own bytes.
 *
 * Shape evidence: the pointer-cell spelling and the separate pointer local for
 * the post-integration group are the shapes the exact siblings of this run use -
 * clearStepAndHeadXYWordsArmCountdown20DispatchCue208AndAdvanceWorkByte1ScenarioSce15ef100_801D2BA8
 * for the arm-cue-advance tail and
 * advanceHeadPairIntegrateStepPairDrawQuadTickCountdownThenResetWorkStateScenarioSce15ef100_801D2C00
 * for the `*(s32*)(ptr + a) += *(s32*)(ptr + b)` integration pairing. The
 * twenty-pass repeat is this body's own loop, which the siblings do not have.
 * Two of this body's shapes are load-bearing and both were measured against the
 * original bytes. The clears run through the first cell load and the loop runs
 * through `step`, a second local assigned from the same pointer after the
 * clears: the original keeps the cell load in $v0 and copies it into $a2 for
 * every later access, and one local shared by clears and loop instead loads
 * the cell straight into $a2, which measures 40 of 50 instructions at 196
 * bytes with its first difference at that load. The arm-and-negate group reads
 * the step pair through a word-typed view (`pairs[0]`/`pairs[1]`), which is what
 * serialises the two read-modify-write arms through one $v0 as the original
 * does; negating the same two words through byte offsets measures 44 of 50 with
 * the two arms issued as raw offset expressions that the scheduler pairs into
 * $v0/$v1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHeadPairIntegrateStepPair20TimesNegateStepPairArmCountdown20DispatchCue208AndAdvanceWorkByte1ScenarioSce15ef100_801D2C94(void) {
  u8* work;
  u8* step;
  u8* arm;
  u8* state;
  s32* pairs;
  u8 i;

  work = D_1F800044;
  *(s32*)(work + 0x0C) = 0;
  *(s32*)(work + 0x10) = 0;
  *(s32*)(work + 0x18) = 0;
  *(s32*)(work + 0x1C) = 0;
  step = work;
  for (i = 0; i < 0x14u; i++) {
    *(s32*)(step + 0x18) += 0x100;
    *(s32*)(step + 0x1C) += 0x200;
    *(s32*)(step + 0x0C) += *(s32*)(step + 0x18);
    *(s32*)(step + 0x10) += *(s32*)(step + 0x1C);
  }
  arm = D_1F800044;
  arm[9] = 0x14;
  pairs = (s32*)(arm + 0x0C);
  pairs[0] = -pairs[0];
  pairs[1] = -pairs[1];
  state = D_1F800044;
  state[1]++;
  dispatchSoundCue(0x208);
}
