#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads the cell afresh before every store of the body, so the source
 * keeps the named-cell form instead of caching the pointer in a local: a store
 * through the pointer may alias the cell itself.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Route step-sign helper of the shared main executable, outside this payload:
 * it classifies a requested route index against the work route byte 0x08 within
 * a wrapped eight-entry window and answers -1 when the bound stands one to four
 * entries above the request and 1 otherwise. It is the helper lifted at
 * 0x801B2D00 as `routeIndexStepSign` in the game payload and it takes one
 * zero-extended byte argument, so both calls here pass a small immediate.
 */
s32 func_801B2D00(u8 index);

/* @source 0x801CF170
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. It takes the unsigned route
 *           index byte 0x08 and compares its distance from the two candidate
 *           route indices 1 and 7, the neighbours of route 0: when the distance
 *           to 7 is the smaller one it asks the shared helper func_801B2D00 for
 *           the step sign towards 7 and stores that answer in byte 0x0B,
 *           otherwise it asks for the sign towards 1 and stores that. Both
 *           paths then store the chosen candidate index in byte 0x03, so the
 *           two branches share one store and differ only in the argument and in
 *           the constant they hand to it. The body then sets the countdown byte
 *           0x09 to 2 and increments the step counter byte 0x02, which re-arms
 *           the work object for the selected route. It takes no arguments,
 *           returns nothing and answers no value. The route byte is loaded once
 *           and both distances are computed from it with the absolute-value
 *           branch pairs the original shows; the pointer cell is re-read before
 *           every store; the first pointer load precedes frame creation and the
 *           0x18-byte frame only keeps $ra at 0x10 across the calls. The body's
 *           address appears once in this payload, as word 0x801CFB9C inside the
 *           sixteen-word pointer run that starts at 0x801CFB84, so it is entry 0
 *           of that interior sub-run and is reached from outside the payload;
 *           its prologue and return path alone place the boundary.
 *
 * Shape note: the 0xCC-byte body is byte-identical to func_801CF170 of the
 * plp012 and plp016 overlays, which held no source when this lift was seeded
 * because their rows were still `asm` then, so the seed comes from the nearest
 * exact lifted sibling instead: the same 0xCC-byte body at 0x801CE448 of
 * plp27a, which differs only in the candidate constants it stores and passes.
 * That sibling is shape evidence, not a clone; the three payloads are the only
 * copies of this body that a scan of the plchar overlay set finds.
 * @status exact
 * @match 100.00
 * @residual none
 */
void pickNearerRouteIndexStoreStepSignAndChosenRouteThenRearmWorkPlcharPlp01500_801CF170(
    void) {
  s32 fromOne;
  s32 fromSeven;

  fromOne = D_1F800044[8] - 1;
  if (fromOne < 0) {
    fromOne = -fromOne;
  }
  fromSeven = D_1F800044[8] - 7;
  if (fromSeven < 0) {
    fromSeven = -fromSeven;
  }
  if (fromSeven < fromOne) {
    D_1F800044[0xB] = func_801B2D00(7);
    D_1F800044[3] = 7;
  } else {
    D_1F800044[0xB] = func_801B2D00(1);
    D_1F800044[3] = 1;
  }
  D_1F800044[9] = 2;
  D_1F800044[2] = D_1F800044[2] + 1;
}
