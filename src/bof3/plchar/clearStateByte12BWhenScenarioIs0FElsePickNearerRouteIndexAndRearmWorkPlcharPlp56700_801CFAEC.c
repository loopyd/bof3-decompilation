#include "bof3/bof3.h"

/* Scenario-state byte of the shared main executable: the signed leading byte of
 * the etc/game/00 scenario-state record. This body only compares it with 0xF and
 * no lift that anchors this byte names that value, so the byte stays
 * address-anchored.
 * @source 0x80146870 @kind data
 */
extern s8 D_80146870;

/* Front-end record pointer cell of the shared main executable. The scenario
 * branch of this body clears this record's state byte 0x12B and touches no
 * other byte of it.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

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
 * @source 0x801B2D00
 */
extern s32 func_801B2D00(u8 index);

/* @source 0x801CFAEC
 * @behavior Overlay step of the front-end work object, reached from outside
 *           this payload: the only word that names it is 0x801CFAEC at
 *           0x801D07D8, entry 0 of this overlay handler run at 0x801D078C, so
 *           the byte-2 dispatcher 0x801CFAA8 that indexes that run calls it
 *           with no arguments and no jal caller exists.
 *           It first reads the signed scenario-state byte 0x80146870: while
 *           that byte reads 0xF the step does nothing to the work object and
 *           only clears the state byte 0x12B of the front-end record published
 *           at 0x80146250, so the work step is skipped entirely in that
 *           scenario. For every other scenario it runs the route re-arm step of
 *           this overlay: it takes the unsigned route index byte 0x08 of the
 *           work object read through the scratchpad pointer cell 0x1F800044 and
 *           compares its distance from the two candidate route indices 3 and 5;
 *           when the distance to 5 is the smaller one it asks the shared helper
 *           func_801B2D00 for the step sign towards 5 and stores that answer in
 *           byte 0x0B, otherwise it asks for the sign towards 3 and stores
 *           that. Both paths then store the chosen candidate index in byte 0x03
 *           through one shared store, which is why the two branches differ only
 *           in the argument and in the constant they hand to the helper. The
 *           body then sets the countdown byte 0x09 to 2 and increments the step
 *           counter byte 0x02, which re-arms the work object for the selected
 *           route. It takes no arguments, returns nothing and answers no value.
 *           The route byte is loaded once and both distances are computed from
 *           it; the pointer cell is re-read before every store because a store
 *           through the pointer may alias the cell itself; this body keeps the
 *           frame first, the 0x18-byte frame only holds the return address at
 *           0x10 across the calls, and the two paths join on one epilogue.
 *
 * Shape note: no span of the extracted payload set is byte-identical to this
 * 0xF4-byte body, because its two internal jump words encode addresses of this
 * own range (0x801CFBD0 and 0x801CFBA0), so only a copy at exactly this address
 * could equal it. Eleven spans of nine plchar payloads carry the same body and
 * each differs in those two words alone, by exactly the offset delta between
 * the two spans: plp247 0x801CEE5C, plp257 0x801CF23C, plp267 0x801CFCE8,
 * plp278 0x801CEC40, plp27a 0x801CEC40, plp457 0x801CEC60, plp467 0x801CF70C,
 * plp478 0x801CE664, plp578 0x801CEA44 and plp678 0x801CF4F0, measured by
 * comparing this body against those payloads. The three already-exact lifts at
 * plp247 0x801CEE5C, plp267 0x801CFCE8 and plp27a 0x801CEC40 supply the
 * statement order, operand forms, offset spelling and declaration style; every
 * literal, stored offset and passed constant here is this body's own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearStateByte12BWhenScenarioIs0FElsePickNearerRouteIndexAndRearmWorkPlcharPlp56700_801CFAEC(
    void) {
  s32 fromThree;
  s32 fromFive;

  if (D_80146870 == 0xF) {
    D_80146250[0x12B] = 0;
  } else {
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
}
