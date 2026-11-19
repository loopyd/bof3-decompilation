#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after every store of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared main-executable resource request helper with one argument, the record
 * resource id this body takes from work byte 0x04. The sibling lifts of this
 * payload declare the same one-argument form.
 */
extern void func_8014D6B8(u32 flag);

/* Shared main-executable resource request helper with a resource id and a
 * count. This body reaches only the pair form (0x50, 2), which the same-address
 * exact body of the plp016 overlay binds with this two-argument prototype.
 */
extern void func_8014D6DC(s32 arg0, s32 arg1);

/* Work-sequence selector of the shared EXE image, outside this payload: it takes
 * a zero-extended byte sequence id and answers a word this body ignores. This
 * target's generated undefined_funcs_auto.txt binds 0x8014D8D4, and the sibling
 * lift at 0x801CF23C of this payload declared the same prototype.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CE628
 * @behavior Work-state handler 1 of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. The body address appears once in
 *           this payload, as the word 0x801CE628 at 0x801D085C, entry 1 of the
 *           twenty-four-word code-pointer run based at 0x801D0858 that the step
 *           dispatch at 0x801CE404 indexes by the work-record byte 0x02, so that
 *           byte selects it. It takes no arguments and returns nothing. Byte
 *           0x08 is the current route index of the eight-entry route window,
 *           byte 0x03 the chosen candidate index, byte 0x09 the countdown, byte
 *           0x0B the step sign of that route walk, byte 0x04 the record resource
 *           id and byte 0x02 the step counter. When the current route index
 *           already equals the chosen index the walk has arrived: the body
 *           requests the record resource through the pair (0x50, 2) of
 *           func_8014D6DC when the id itself reads 0x50 and as the bare id
 *           through func_8014D6B8 for every other id, then increments the step
 *           counter byte 0x02, which moves the dispatch of 0x801CE404 to entry
 *           2 of the same run. Otherwise it stores the countdown byte 0x09 one
 *           lower and, when that reaches zero, puts it back to 2, adds the step
 *           sign byte 0x0B into the route index byte 0x08, masks the index into
 *           the eight-entry window with 7 and selects the sequence of the new
 *           index through func_8014D8D4; while the countdown stays nonzero that
 *           path writes nothing else. The 0x18-byte frame only keeps $ra at
 *           0x10 across the calls and the first pointer load precedes frame
 *           creation. The route test reads both bytes from that first pointer
 *           load and the countdown decrement reuses the same value, while the
 *           three store groups after it re-read the cell. The constant 0x50 is
 *           materialised in the delay slot of the route test, the constant 2 in
 *           the delay slot of the branch that skips the expiry path, and the
 *           step counter increment issues in the delay slot of the jump to the
 *           epilogue. No argument is set up before the one-argument request at
 *           0x801CE670: that arm spends the byte-4 load of its own comparison as
 *           the argument, so it adds no access of its own.
 *
 * Shape note: the 0x104-byte span is byte-identical, its own epilogue jump word
 * included, to the same-address span of the plp012, plp015, plp016, plp025,
 * plp026 and plp034 overlays, proven by one cmp of the extracted spans
 * (sha256 64bb146f7364647eea3baaa3affe312578bf8a21e65105af84a1986aa6a6cab0). The
 * plp016 row of that address was already lifted exact, so its body seeded this
 * statement order, operand forms and offset spelling, with this target own
 * literals and bindings re-derived from the bytes above. The same-target exact
 * body at 0x801CF23C shares the countdown and route-step arm statement for
 * statement and differs only in its arrived arm, so it supplied the spelling of
 * that arm which this body does not carry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestWorkResourceByte4WithCount2For50ThenAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp05600_801CE628(
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
