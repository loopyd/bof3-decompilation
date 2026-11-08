#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after every store of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Work-sequence selector of the shared EXE image, outside this payload: it takes
 * a zero-extended byte sequence id and answers a word this body ignores. This
 * target's generated undefined_funcs_auto.txt binds 0x8014D8D4, and the exact
 * lifts of the same shape in the plp016 and plp257 overlays declared the same
 * prototype. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CEB38
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. Byte 0x08 is the current route
 *           index of the eight-entry route window, byte 0x03 the chosen
 *           candidate index that the re-arm step stores, byte 0x09 the
 *           countdown, byte 0x02 the step counter and byte 0x0B the step sign of
 *           that route walk. When the current route index already equals the
 *           chosen index the walk has arrived, so the body selects that route
 *           arrival sequence through the shared selector func_8014D8D4,
 *           sequence 0x40 for route 3 and sequence 0x41 for route 5, and no
 *           sequence at all for any other route. It then increments the step
 *           counter byte 0x02. Otherwise it stores the countdown byte 0x09 one
 *           lower and, when that reaches zero, puts it back to 2, adds the step
 *           sign byte 0x0B into the route index byte 0x08, masks that index into
 *           the eight-entry window with 7 and selects the sequence of the new
 *           route index through the same selector. It takes no arguments,
 *           returns nothing and answers no value. The route index and the chosen
 *           index come from one pointer load, so the arrived branch compares
 *           them once and both route tests reuse that same loaded byte; the
 *           increment of byte 0x02 issues in the delay slot of the jump to the
 *           epilogue, the countdown store reuses the pointer load of the first
 *           comparison, and the constant 2 is materialised in the delay slot of
 *           the branch that skips the expiry path. The 0x18-byte frame only
 *           keeps $ra at 0x10 across the calls, and the first pointer load
 *           precedes frame creation.
 *
 * The 0xFC-byte body measured byte-identical to the plp016 lift at 0x801CF610
 * apart from the two absolute jump words at body offsets 0x34 and 0x60, each of
 * which encodes that body's own epilogue address, so that lift was the seed and
 * a byte-identical clone was not available. This payload holds a second copy of
 * the same step body at 0x801CE514 which pairs the same sequences but reaches
 * the shared selector from a branch past it instead of the taken one used here,
 * so that copy is shape evidence only. The only word of this payload that names
 * this body is the pointer 0x801CEB38 stored at 0x801CFAD4, word 7 of the
 * twenty-three-word code pointer run based at 0x801CFAB8, and no word of the
 * payload encodes a jump to it, so an in-payload dispatcher enters it and it has
 * no direct caller. The run also holds the 0x801CE514 copy at its word 1, so one
 * payload step sequence advances through both variants.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp57800_801CEB38(
    void) {
  if (D_1F800044[8] == D_1F800044[3]) {
    switch (D_1F800044[8]) {
      case 3:
        func_8014D8D4(0x40);
        break;
      case 5:
        func_8014D8D4(0x41);
        break;
    }
    D_1F800044[2] = D_1F800044[2] + 1;
  } else {
    D_1F800044[9] = D_1F800044[9] - 1;
    if (D_1F800044[9] == 0) {
      D_1F800044[9] = 2;
      D_1F800044[8] = D_1F800044[8] + D_1F800044[0xB];
      D_1F800044[8] = D_1F800044[8] & 7;
      func_8014D8D4(D_1F800044[8]);
    }
  }
}
