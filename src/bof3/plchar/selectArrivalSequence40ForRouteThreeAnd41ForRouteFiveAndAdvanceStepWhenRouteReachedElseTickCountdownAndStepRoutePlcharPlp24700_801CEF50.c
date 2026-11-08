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
 * target's generated undefined_funcs_auto.txt binds the address. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CEF50
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload, as the word
 *           0x801CFB20, entry 9 of the twenty-word handler run that fills the
 *           trailing read-only block at 0x801CFAFC. Byte 0x08 is the current
 *           route index of the eight-entry route window, byte 0x03 the chosen
 *           candidate index that the re-arm step stores, byte 0x09 the
 *           countdown, byte 0x02 the step counter and byte 0x0B the step sign of
 *           that route walk. When the current route index already equals the
 *           chosen index the walk has arrived, so the body selects that route
 *           arrival sequence through the shared selector func_8014D8D4:
 *           sequence 0x40 for route 3 and sequence 0x41 for route 5, and no
 *           sequence at all for any other route, which is why the lookup is a
 *           two-case route switch rather than an if/else pair. It then
 *           increments the step counter byte 0x02. Otherwise it stores the
 *           countdown byte 0x09 one lower, and when that reaches zero puts it
 *           back to 2, adds the step sign byte 0x0B into the route index byte
 *           0x08, masks that index into the eight-entry window with 7 and
 *           selects the sequence of the new route index through the same
 *           selector. It takes no arguments, returns nothing and answers no
 *           value. The 0x18-byte frame only keeps the return address at 0x10
 *           across the calls, the first pointer load precedes frame creation,
 *           the increment of byte 0x02 issues in the delay slot of the jump to
 *           the epilogue and the constant 2 is materialised in the delay slot of
 *           the branch that skips the expiry path. The bytes are those of the
 *           exact lift at 0x801CED34 of the plp27a overlay, whose body lists the
 *           same two route arms in the same order: a cmp of the two 252-byte
 *           spans reported 250 of 252 bytes equal, the two words that differed
 *           being the jump words that encode each body own epilogue and skip
 *           addresses, each shifted by the 0x21C offset between the two spans.
 *           That twin was therefore shape evidence rather than a clone, and this
 *           body was transposed from it with this payload literals.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequence40ForRouteThreeAnd41ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp24700_801CEF50(
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
