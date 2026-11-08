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
 * refreshLayersThenSelectWorkSequenceFromByte8HalfArmCountdownAndAdvanceWorkByte3PlcharPlp01600_801D0060
 * of this target declares the same prototype, and this target's generated
 * undefined_funcs_auto.txt binds the address. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF610
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as word
 *           0x801CF610 at 0x801D0660, entry 12 of the thirty-entry handler run
 *           based at 0x801D0630 (the `textbin` row T_801D0630). Byte 0x08 is the
 *           current route index of the eight-entry route window, byte 0x03 the
 *           chosen candidate index that the re-arm step stores, byte 0x09 the
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
 *           takes no arguments, returns nothing and answers no value. The 0x18-byte
 *           frame only keeps $ra at 0x10 across the calls, the increment of byte
 *           0x02 issues in the delay slot of the jump to the epilogue, the
 *           countdown store reuses the pointer load of the first comparison, and
 *           the constant 2 is materialised in the delay slot of the branch that
 *           skips the expiry path. The first pointer load precedes frame
 *           creation, and the route index of the arrived branch and the chosen
 *           index come from that one load.
 *
 * Shape note: the 0xFC-byte body is byte-identical to the body at 0x801CED34 of
 * the plp27a overlay apart from the two jump words that encode each body's own
 * epilogue address, so that sibling is shape evidence rather than a clone. The
 * body at 0x801CF610 of the plp015 overlay pairs the same sequences but reaches
 * the shared call from a `bne` past it instead of the taken `beq` of this body,
 * and the plp012 copy swaps the two sequences, so both are shape evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp01600_801CF610(
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
