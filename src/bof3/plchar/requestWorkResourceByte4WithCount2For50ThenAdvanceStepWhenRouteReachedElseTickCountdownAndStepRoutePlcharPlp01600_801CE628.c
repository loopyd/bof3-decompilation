#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after every store of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared main-executable resource request helper with one argument. The sibling
 * lifts of this target declare the same prototype and hand it the resource id
 * the record wants next.
 */
extern void func_8014D6B8(s32 arg0);

/* Shared main-executable resource request helper with a resource id and a
 * count. The shipped body masks its first argument with 0x80 and the AREA030
 * lift reaches the same two-argument form, so the pair prototype is the
 * evidenced one. This payload reaches it at 0x801CE660 only.
 */
extern void func_8014D6DC(s32 arg0, s32 arg1);

/* Work-sequence selector of the shared EXE image, outside this payload. It takes
 * a zero-extended byte sequence id and answers a word this body ignores, and the
 * exact sibling 0x801CF610 of this target declares the same prototype.
 */
extern u32 func_8014D8D4(u8 arg0);

/* @source 0x801CE628
 * @behavior Overlay entry over the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. No word and no jump of this
 *           payload names the body address, so the shared EXE reaches it from
 *           outside, and it takes no arguments and returns nothing. Byte 0x08 is
 *           the current route index of the eight-entry route window, byte 0x03
 *           the chosen candidate index, byte 0x02 the step counter, byte 0x09
 *           the countdown, byte 0x0B the step sign of that walk and byte 0x04
 *           the record resource id; the exact sibling 0x801CF610 of this target
 *           fixes those roles for the same cell and the same walk, while the
 *           resource id role comes from this body alone. When the current route
 *           index already equals the chosen index the walk has arrived: the body
 *           requests the record resource again, as the pair (0x50, 2) through
 *           func_8014D6DC when the id itself reads 0x50 and as the bare id
 *           through func_8014D6B8 for every other id, and then increments the
 *           step counter byte 0x02. Otherwise it stores the countdown byte 0x09
 *           one lower and, when that reaches zero, puts it back to 2, adds the
 *           step sign byte 0x0B into the route index byte 0x08, masks the index
 *           into the eight-entry window with 7 and selects the sequence of the
 *           new index through func_8014D8D4; while the countdown stays nonzero
 *           that path writes nothing else. The 0x18-byte frame only keeps $ra at
 *           0x10 across the calls, the first pointer load precedes frame
 *           creation, the constant 0x50 is materialised in the delay slot of the
 *           route test, the countdown decrement reuses the pointer load of that
 *           test, the constant 2 is materialised in the delay slot of the branch
 *           that skips the expiry path, and the step counter increment issues in
 *           the delay slot of the jump to the epilogue. The arrived arm spends
 *           the byte-4 load of the comparison as its own request argument, so it
 *           adds no access of its own.
 *
 * Shape note: the 0x104-byte body is byte-identical, including its own epilogue
 * jump word, to the same-address body of the plp012, plp015, plp025, plp026,
 * plp034 and plp056 overlays, but every one of those rows is still `asm`, so no
 * lifted representative exists and this body seeds from the nearest exact
 * sibling instead: 0x801CF610 of this target reproduces the whole countdown and
 * route-step arm statement for statement and differs only in its arrived arm,
 * which selects sequence 0x40 or 0x41 through func_8014D8D4 where this body
 * requests the record resource.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestWorkResourceByte4WithCount2For50ThenAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp01600_801CE628(
    void) {
  if (D_1F800044[8] == D_1F800044[3]) {
    if (D_1F800044[4] == 0x50) {
      func_8014D6DC(0x50, 2);
    } else {
      func_8014D6B8(D_1F800044[4]);
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
