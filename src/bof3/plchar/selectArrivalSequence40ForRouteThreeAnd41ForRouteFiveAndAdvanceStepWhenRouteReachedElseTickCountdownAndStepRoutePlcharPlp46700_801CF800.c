#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after every store of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Work-sequence selector of the shared EXE image, outside this payload: it takes
 * a zero-extended byte sequence id and answers a word this body ignores. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt,
 * and the other lifted rows of this payload declare the same prototype.
 */
u32 func_8014D8D4(u8 arg0);

/* Seeded from the lift of the same body at plp025 0x801CFA34, which had gated
 * exact when this seed was written. This 0xFC-byte span compared to that range
 * over the payload bytes with exactly three differing words: the two jump words
 * whose targets are internal to each body, at body offsets 0x34 and 0x60, and the
 * route test at body offset 0x2C, where that copy negates the second test and
 * this one branches forward into the shared selector call. Each of the two jump
 * targets lands on the same body offset in both spans and the remaining sixty
 * words stay equal, so one body and one statement order are proven while the
 * branch difference leaves no clone. The same body also sat word-identical at
 * plp026 0x801CFA34 except those two jump words, and that row was still an `asm`
 * row in its own target when this seed was written, so it carried no source to
 * copy. The branch topology below is written from this span's own bytes, which
 * test route 3 first and branch forward into the one shared selector call.
 */
/* @source 0x801CF800
 * @behavior Overlay step of the front-end work object read through the scratchpad
 *           pointer cell 0x1F800044, reached from outside this payload: the body
 *           address appears once in the payload, as word 0x801D03F0, entry 17 of
 *           the twenty-eight-word handler run based at 0x801D03AC and index 1 of
 *           the base 0x801D03EC that the work-byte-2 dispatcher 0x801CF6C8
 *           enters after scaling work byte 0x02 by four, so no jump in this
 *           payload names it. Byte 0x08 is the current route index of the
 *           eight-entry route window, byte 0x03 the chosen candidate index, byte
 *           0x09 the countdown, byte 0x02 the step counter and byte 0x0B the step
 *           sign of that route walk. When the current route index already equals
 *           the chosen index the walk has arrived, so the body selects that route
 *           arrival sequence through the shared selector func_8014D8D4, sequence
 *           0x40 for route 3 and sequence 0x41 for route 5, and no sequence at all
 *           for any other route, so the lookup tests route 3 first and asks about
 *           route 5 only when that test fails. It then increments the step counter
 *           byte 0x02. Otherwise it stores the countdown byte 0x09 one lower and,
 *           when that reaches zero, puts it back to 2, adds the step sign byte
 *           0x0B into the route index byte 0x08, masks that index into the
 *           eight-entry window with 7 and selects the sequence of the new route
 *           index through the same selector. It takes no arguments, returns
 *           nothing and answers no value. The route index and the chosen index
 *           come from one pointer load, so the arrived branch compares them once;
 *           the constant 3 enters in the delay slot of that comparison, both route
 *           tests branch forward into the one selector call that carries the
 *           argument register, the increment of byte 0x02 issues in the delay slot
 *           of the jump to the epilogue, the countdown store reuses the pointer
 *           load of the first comparison, and the constant 2 is materialised in
 *           the delay slot of the branch that skips the expiry path. The
 *           0x18-byte frame only keeps the return address at 0x10 across the
 *           calls, and the first pointer load precedes frame creation.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp46700_801CF800(
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
