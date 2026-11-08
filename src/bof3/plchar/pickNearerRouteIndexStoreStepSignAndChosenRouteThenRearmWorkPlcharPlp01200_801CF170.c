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

/* @source 0x801CF170
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. It takes the unsigned route
 *           index byte 0x08 and compares its distance from the two candidate
 *           route indices 1 and 7: when the distance to 7 is the smaller one it
 *           asks the shared helper func_801B2D00 for the step sign towards 7
 *           and stores that answer in byte 0x0B, otherwise it asks for the sign
 *           towards 1 and stores that. Both paths then store the chosen
 *           candidate index in byte 0x03, so the two branches share one store
 *           and differ only in the argument they pass and in the constant they
 *           hand to it. The body then sets the countdown byte 0x09 to 2 and
 *           increments the step counter byte 0x02, which re-arms the work object
 *           for the selected route. It takes no arguments, returns nothing and
 *           answers no value. The route byte is loaded once and both distances
 *           are computed from it; the pointer cell is re-read before every store
 *           because a store through the pointer may alias the cell itself; the
 *           first pointer load precedes frame creation and the 0x18-byte frame
 *           only keeps the return address at 0x10 across the calls. The
 *           0xCC-byte original range is byte-identical to the span at 0x801CF170
 *           of the plp015/00 and plp016/00 payloads, whose rows are still asm
 *           and so hold no source to clone, and its nearest exact sibling is the
 *           plp27a lift at 0x801CE448, whose 0xCC body differs only in the six
 *           words that carry its candidate indices 3 and 5 and in the one
 *           intra-body jump word that encodes its own shared-store address, so
 *           the statement order, operand forms and declaration style here are
 *           that body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void pickNearerRouteIndexStoreStepSignAndChosenRouteThenRearmWorkPlcharPlp01200_801CF170(
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
