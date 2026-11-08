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
 * keeps the named-cell form instead of caching the pointer in a local.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Route step-sign helper of the shared main executable, outside this payload:
 * it classifies a requested route index against the work route byte 0x08 within
 * a wrapped eight-entry window and answers -1 when the bound stands one to four
 * entries above the request and 1 otherwise. Both calls here pass a small
 * immediate.
 * @source 0x801B2D00
 */
extern s32 func_801B2D00(u8 index);

/* @source 0x801CFCE8
 * @behavior Overlay step of the front-end work object, reached through this
 *           payload's thirty-word code-pointer run at 0x801D0988: its address
 *           is the word stored at 0x801D09CC, entry 17 of that run, and the
 *           byte-2 work dispatcher at 0x801CEE28 indexes the run from
 *           0x801D09A8, which places this body at index 9 there. The body
 *           increments work byte 2, so its own end re-arms the byte-2 dispatch
 *           to read entry 10, the word 0x801CFDDC at 0x801D09D0. It first reads
 *           the signed scenario-state byte 0x80146870: while that byte reads
 *           0xF the step does nothing to the work object and only clears the
 *           state byte 0x12B of the front-end record published at 0x80146250,
 *           so the work step is skipped entirely in that scenario. For every
 *           other scenario it runs the route re-arm step of this overlay: it
 *           takes the unsigned route index byte 0x08 of the work object read
 *           through the scratchpad pointer cell 0x1F800044 and compares its
 *           distance from the two candidate route indices 3 and 5; when the
 *           distance to 5 is the smaller one it asks the shared helper
 *           func_801B2D00 for the step sign towards 5 and stores that answer in
 *           byte 0x0B, otherwise it asks for the sign towards 3 and stores
 *           that. Both paths then store the chosen candidate index in byte 0x03
 *           through one shared store at 0x801CFD9C, which is why the two
 *           branches differ only in the argument and in the constant they hand
 *           to the helper. The body then sets the countdown byte 0x09 to 2 and
 *           increments the step counter byte 0x02, which re-arms the work
 *           object for the selected route. It takes no arguments, returns
 *           nothing and answers no value. The route byte is loaded once and
 *           both distances are computed from it; the pointer cell is re-read
 *           before every store because a store through the pointer may alias
 *           the cell itself; the first pointer-cell load of the scenario test
 *           sits before frame creation, the 0x18-byte frame only keeps the
 *           return address at 0x10 across the calls, and the two paths join on
 *           one epilogue at 0x801CFDCC. This body is the body the plp27a
 *           overlay carries at 0x801CEC40: all 61 words are equal except the
 *           two `j` words at +0x1C and +0x84, each of which encodes a join
 *           address of its own payload, so the clone adapter has no
 *           byte-identical sibling to copy and the body was transposed from
 *           that exact lift.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearStateByte12BWhenScenarioIs0FElsePickNearerRouteIndexAndRearmWorkPlcharPlp26700_801CFCE8(
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
