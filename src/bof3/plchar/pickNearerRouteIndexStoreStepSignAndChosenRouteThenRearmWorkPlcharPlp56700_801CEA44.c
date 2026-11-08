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
 * zero-extended byte argument, so both calls here pass a small immediate. */
s32 func_801B2D00(u8 index);

/* @source 0x801CEA44
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. It takes the unsigned route
 *           index byte 0x08 and compares its distance from the two candidate
 *           route indices 3 and 5: when the distance to 5 is the smaller one it
 *           asks the shared helper func_801B2D00 for the step sign towards 5
 *           and stores that answer in byte 0x0B, otherwise it asks for the sign
 *           towards 3 and stores that. Both paths then store the chosen
 *           candidate index in byte 0x03, so the two branches share one store
 *           and differ only in the argument and in the constant they hand to
 *           it. The body then sets the countdown byte 0x09 to 2 and increments
 *           the step counter byte 0x02, which re-arms the work object for the
 *           selected route. It takes no arguments, returns nothing and answers
 *           no value. The route byte is loaded once and both distances are
 *           computed from it; the pointer cell is re-read before every store
 *           because a store through the pointer may alias the cell itself; the
 *           first pointer load precedes frame creation and the 0x18-byte frame
 *           only keeps $ra at 0x10 across the calls. The address of this body
 *           is entry 0 of this overlay's handler table 0x801D07A4, which the
 *           dispatcher at 0x801CEA00 indexes by work byte 0x02, so this step is
 *           reached from outside the payload and its prologue and return path
 *           alone place the boundary.
 *
 * Shape note: no other span of the extracted payload set is byte-identical to
 * this 0xCC-byte body. The nearest sibling is the same body in this payload at
 * 0x801CE448, which differs in exactly one word, the intra-body jump at span
 * offset 0x64 that encodes its own shared-store address 0x801CEAD4 against
 * 0x801CE4D8. Beyond that the same body with the same candidate constants 3 and
 * 5 sits in 33 further spans of 18 payloads, so the statement order, operand
 * forms and declaration style here are those already-exact sibling lifts and
 * the literals and offsets are this body's own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void pickNearerRouteIndexStoreStepSignAndChosenRouteThenRearmWorkPlcharPlp56700_801CEA44(
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
