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
 * siblings of the same shape in the plp015 and plp025 overlays declared the same
 * prototype. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CED0C
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as word
 *           0x801CFEF4, entry 6 of the twenty-three-entry code pointer run that
 *           starts at 0x801CFEDC and whose base no word of this payload names.
 *           Byte 0x08 is the current route index of the eight-entry route
 *           window, byte 0x03 the chosen candidate index that the re-arm step
 *           stores, byte 0x09 the countdown, byte 0x02 the step counter and byte
 *           0x0B the step sign of that route walk. When the current route index
 *           already equals the chosen index the walk has arrived, so the body
 *           selects that route arrival sequence through the shared selector
 *           func_8014D8D4, sequence 0x40 for route 3 and sequence 0x41 for
 *           route 5, and no sequence at all for any other route, so the lookup
 *           tests route 3 first and asks about route 5 only when that test
 *           fails. It then increments the step counter byte 0x02. Otherwise it
 *           stores the countdown byte 0x09 one lower and, when that reaches
 *           zero, puts it back to 2, adds the step sign byte 0x0B into the route
 *           index byte 0x08, masks that index into the eight-entry window with 7
 *           and selects the sequence of the new route index through the same
 *           selector. It takes no arguments, returns nothing and answers no
 *           value. The route index and the chosen index come from one pointer
 *           load, so the arrived branch compares them once and both route tests
 *           reuse that same loaded byte; the increment of byte 0x02 issues in
 *           the delay slot of the jump to the epilogue, the countdown store
 *           reuses the pointer load of the first comparison, and the constant 2
 *           is materialised in the delay slot of the branch that skips the
 *           expiry path. The 0x18-byte frame only keeps $ra at 0x10 across the
 *           calls, and the first pointer load precedes frame creation.
 *
 * Shape note: the 0xFC-byte body measured word-identical to the copies at
 * 0x801CFA34 of the plp025 overlay, 0x801CF610 of plp015, 0x801CF23C of plp056,
 * 0x801CE730 of plp457 and 0x801CE514 of plp567 and of plp578 except the two
 * jump words at body offsets 0x34 and 0x60, each of which shifted with the
 * address delta between the two spans, so those copies were shape evidence and
 * this source was seeded from the plp025 one, which paired the sequences the
 * same way.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp25700_801CED0C(
    void) {
  if (D_1F800044[8] == D_1F800044[3]) {
    if (D_1F800044[8] != 3) {
      if (D_1F800044[8] == 5) {
        func_8014D8D4(0x41);
      }
    } else {
      func_8014D8D4(0x40);
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
