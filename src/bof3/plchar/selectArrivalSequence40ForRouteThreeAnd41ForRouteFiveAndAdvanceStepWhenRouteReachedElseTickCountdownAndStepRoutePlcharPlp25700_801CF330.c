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
 * selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp25700_801CED0C
 * of this target declares the same prototype, and this target's generated
 * undefined_funcs_auto.txt binds the address. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF330
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as word
 *           0x801CF330 at 0x801CFF0C, entry 12 of the twenty-three-entry handler
 *           run based at 0x801CFEDC, whose entries 0 through 15 sit in the
 *           `textbin` row func_801CFEDC. This payload's own byte-2 dispatcher at
 *           0x801CF1F8 indexes that run from 0x801CFF08, which is entry 11, so
 *           work byte 2 value 1 selects this body. Byte 0x08 is the current
 *           route index of the eight-entry route window, byte 0x03 the chosen
 *           candidate index that the re-arm step stores, byte 0x09 the
 *           countdown, byte 0x02 the step counter and byte 0x0B the step sign of
 *           that route walk. When the current route index already equals the
 *           chosen index the walk has arrived, so the body selects that route
 *           arrival sequence through the shared selector func_8014D8D4,
 *           sequence 0x40 for route 3 and sequence 0x41 for route 5, and no
 *           sequence at all for any other route, which is why the lookup is a
 *           two-case route switch rather than an if/else pair. It then
 *           increments the step counter byte 0x02. Otherwise it stores the
 *           countdown byte 0x09 one lower and, when that reaches zero, puts it
 *           back to 2, adds the step sign byte 0x0B into the route index byte
 *           0x08, masks that index into the eight-entry window with 7 and selects
 *           the sequence of the new route index through the same selector. It
 *           takes no arguments, returns nothing and answers no value. The
 *           0x18-byte frame only keeps $ra at 0x10 across the calls, the first
 *           pointer load precedes frame creation, the arrived branch takes the
 *           route index and the chosen index from that one load, the increment
 *           of byte 0x02 issues in the delay slot of the jump to the epilogue,
 *           the countdown store reuses the pointer load of the first comparison,
 *           and the constant 2 is materialised in the delay slot of the branch
 *           that skips the expiry path.
 *
 * Shape note: the 0xFC-byte body is instruction-identical to the copies at
 * 0x801CF610 of the plp016 overlay, 0x801CFA34 of plp026, 0x801D0160 of plp034,
 * 0x801CF838 of plp056 and 0x801CED0C and 0x801CEF50 of plp247 apart from the two
 * jump words that carry each body's own absolute target address, so those copies
 * are shape evidence rather than clones and this source was seeded from the
 * plp016 one, which pairs the sequences the same way. The copy at 0x801CED0C of
 * this target pairs the same sequences but reaches the shared call from a `bne`
 * past it instead of the taken `beq` of this body, so it is shape evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp25700_801CF330(
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
