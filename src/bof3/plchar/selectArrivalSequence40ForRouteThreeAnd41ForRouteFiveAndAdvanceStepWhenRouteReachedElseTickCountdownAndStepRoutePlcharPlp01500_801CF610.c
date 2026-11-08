#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after every store of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Work-sequence selector of the shared EXE image, outside this payload: it takes
 * a zero-extended byte sequence id and answers a word this body ignores. The
 * exact sibling
 * selectArrivalSequence40ForRouteOneAnd41ForRouteSevenAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp01500_801CF23C
 * of this target declares the same prototype, and this target's generated
 * undefined_funcs_auto.txt binds the address. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF610
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as word
 *           0x801CFBB4, entry 12 of the sixteen-word handler run that starts at
 *           0x801CFB84. Byte 0x08 is the current route index of the eight-entry
 *           route window, byte 0x03 the chosen candidate index that the re-arm
 *           step stores, byte 0x09 the countdown, byte 0x02 the step counter and
 *           byte 0x0B the step sign of that route walk. When the current route
 *           index already equals the chosen index the walk has arrived, so the
 *           body selects that route arrival sequence through the shared selector
 *           func_8014D8D4, sequence 0x40 for route 3 and sequence 0x41 for route
 *           5, and no sequence at all for any other route, so the lookup tests
 *           route 3 first and asks about route 5 only when that test fails. It
 *           then increments the step counter byte 0x02. Otherwise it stores the
 *           countdown byte 0x09 one lower and, when that reaches zero, puts it
 *           back to 2, adds the step sign byte 0x0B into the route index byte
 *           0x08, masks that index into the eight-entry window with 7 and selects
 *           the sequence of the new route index through the same selector. It
 *           takes no arguments, returns nothing and answers no value. The route
 *           index and the chosen index come from one pointer load, so the
 *           arrived branch compares them once and both route tests reuse that
 *           same loaded byte; the increment of byte 0x02 issues in the delay slot
 *           of the jump to the epilogue, the countdown store reuses the pointer
 *           load of the first comparison, and the constant 2 is materialised in
 *           the delay slot of the branch that skips the expiry path. The 0x18-byte
 *           frame only keeps $ra at 0x10 across the calls, and the first pointer
 *           load precedes frame creation.
 *
 * Shape note: the 0xFC-byte body is byte-identical to the body at 0x801CF610 of
 * the plp012 overlay, which held no source when this lift was seeded because its
 * row was still `asm` there, so that copy is shape evidence rather than a clone;
 * exactly two words differ and both are the sequence immediates, 0x41 for route
 * 3 there against 0x40 here. The body at 0x801CED34 of the plp27a overlay pairs
 * the same route 3 with 0x40 and route 5 with 0x41 and differs in three words
 * only, two jump words that encode each body own epilogue and skip addresses plus
 * the second route test, which that copy writes as a `beq` onto its call where
 * this body writes a `bne` past the call. That inverted test is why this seed
 * spells the arrived lookup as a negated outer test on route 3 whose fall-through
 * arm asks about route 5, and not as the `switch` of the route 1 and route 7
 * sibling at 0x801CF23C of this target or of the route 3 and route 5 sibling in
 * plp27a: both switch seeds compile to a `beq`/`beq`/`j` triple that this body
 * does not show, and the first measured if/else-if seed of the same two arms
 * compiled the mirror image of this body, branching on route 3 taken and jumping
 * to the merged call from that arm instead of from the route 5 arm.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp01500_801CF610(
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
