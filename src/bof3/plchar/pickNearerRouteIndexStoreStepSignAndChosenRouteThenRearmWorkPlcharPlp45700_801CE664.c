#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads the cell afresh before every store of the body, so the source
 * keeps the named-cell form instead of caching the pointer in a local.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Route step-sign helper of the shared main executable, outside this payload:
 * it classifies a requested route index against the work route byte 0x08 within
 * a wrapped eight-entry window and answers -1 when the bound stands one to four
 * entries above the request and 1 otherwise. It is the helper lifted at
 * 0x801B2D00 as `routeIndexStepSign` in the game payload and it takes one
 * zero-extended byte argument, so both calls here pass a small immediate. */
s32 func_801B2D00(u8 index);

/* The 0xCC-byte body is identical to the exact lift at 0x801CE448 in the plp27a
 * overlay except for its one internal jump displacement (0x801CE6F4 here,
 * 0x801CE4D8 there), which moves with the span, so the body below transcribes
 * that sibling's shape with every constant re-read from this payload's bytes.
 *
 * @source 0x801CE664
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. It takes the unsigned route
 *           index byte 0x08, loaded once, and compares its distance from the two
 *           candidate route indices 3 and 5: when the distance to 5 is the
 *           smaller one it asks the shared helper func_801B2D00 for the step
 *           sign towards 5 and stores that answer in byte 0x0B, otherwise it
 *           asks for the sign towards 3 and stores that. Both paths then store
 *           the chosen candidate index in byte 0x03, so the two arms share one
 *           store and differ only in the immediate they pass and in the constant
 *           they hand to it. The body closes by setting the countdown byte 0x09
 *           to 2 and incrementing the step counter byte 0x02, which re-arms the
 *           work object for the selected route. It takes no arguments, returns
 *           nothing and answers no value. Both distances derive from the one
 *           route byte; the pointer cell is re-read before every store because a
 *           store through the pointer may alias the cell itself; the first
 *           pointer load precedes frame creation and the 0x18-byte frame only
 *           keeps $ra at 0x10 across the calls. Its only in-image reference is
 *           the word 0x801CE664 at 0x801CF90C, entry 3 of this overlay's handler
 *           run at 0x801CF900, so the loader that steps that run enters it: it
 *           has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void pickNearerRouteIndexStoreStepSignAndChosenRouteThenRearmWorkPlcharPlp45700_801CE664(
    void) {
  s32 fromThree;
  s32 fromFive;

  fromThree = D_1F800044[8] - 3;
  if (fromThree < 0) {
    fromThree = -fromThree;
  }
  fromFive = D_1F800044[8] - 5;
  if (fromFive < 0) {
    fromFive = -fromFive;
  }
  if (fromFive < fromThree) {
    D_1F800044[0xB] = func_801B2D00(5);
    D_1F800044[3] = 5;
  } else {
    D_1F800044[0xB] = func_801B2D00(3);
    D_1F800044[3] = 3;
  }
  D_1F800044[9] = 2;
  D_1F800044[2] = D_1F800044[2] + 1;
}
