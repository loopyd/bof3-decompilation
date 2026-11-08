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
 * exact siblings
 * selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp27a00_801CED34
 * and
 * selectArrivalSequenceAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp27a00_801CE514
 * declare the same prototype, and this target's generated
 * undefined_funcs_auto.txt binds the address. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CF23C
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as word
 *           0x801CFBA0, entry 7 of the sixteen-word handler run that starts at
 *           0x801CFB84. Byte 0x08 is the current route
 *           index of the eight-entry route window, byte 0x03 the chosen
 *           candidate index that the re-arm step stores, byte 0x09 the
 *           countdown, byte 0x02 the step counter and byte 0x0B the step sign of
 *           that route walk. The re-arm step of this overlay picks the nearer of
 *           route indices 1 and 7, so the walk covers the two neighbours of
 *           route 0. When the current route index already equals the chosen
 *           index the walk has arrived, so the body selects that route arrival
 *           sequence through the shared selector func_8014D8D4: sequence 0x40
 *           for route 1 and sequence 0x41 for route 7, and no sequence at all
 *           for any other route, which is why the lookup is a two-case route
 *           switch rather than an if/else pair. It then increments the step
 *           counter byte 0x02. Otherwise it stores the countdown byte 0x09 one
 *           lower, and when that reaches zero puts it back to 2, adds the step
 *           sign byte 0x0B into the route index byte 0x08, masks that index into
 *           the eight-entry window with 7 and selects the sequence of the new
 *           route index through the same selector. It takes no arguments,
 *           returns nothing and answers no value. The route index and the chosen
 *           index come from one pointer load, so the arrived branch compares
 *           them once and the switch re-reads the route byte only after the
 *           first store through the pointer forces a fresh cell read; the
 *           0x18-byte frame only keeps $ra at 0x10 across the calls, the
 *           increment of byte 0x02 issues in the delay slot of the jump to the
 *           epilogue and the constant 2 is materialised in the delay slot of the
 *           branch that skips the expiry path.
 *
 * Shape note: the 0xFC-byte body differs from the exact sibling at 0x801CED34 of
 * the plp27a overlay in four words only, the two switch constants and the two
 * jump words that encode each body own epilogue and skip addresses; a positional
 * comparison of the two instruction streams reports 59 of 63 words equal, and
 * the sibling pairs route 3 with 0x40 and route 5 with 0x41 in the same arm
 * order, so the seed copies its statement order and writes this body own route
 * constants and the sequence each case passes. That sibling is shape evidence,
 * not a clone. The 0x801CE514 sibling of the same overlay pairs route 5 with
 * 0x40 and lists route 5 first, so it compiles to the jump-over arm order and is
 * not usable as this seed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequence40ForRouteOneAnd41ForRouteSevenAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp01500_801CF23C(
    void) {
  if (D_1F800044[8] == D_1F800044[3]) {
    switch (D_1F800044[8]) {
      case 1:
        func_8014D8D4(0x40);
        break;
      case 7:
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
