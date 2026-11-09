#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell; the original re-materialises its
 * address for every access, so it keeps the symbol form.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Coordinate-pair to depth helper of the shared main executable: it takes a
 * signed 16.16 word pair as its two coordinates and answers their distance as
 * a halfword. The original adds 0x240 to that answer with a plain 32-bit add
 * and shows no widening or masking, so this overlay sees a word return.
 * @source 0x8015477C
 */
extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801FD748
 * @behavior Handler 2 of the three-word handler table at 0x801FE46C, which
 *           func_801FD34C indexes with work byte 0x04 of the scratchpad work
 *           object that pointer cell 0x1F800044 publishes. That dispatcher
 *           reads the byte with lbu, scales it by four, loads the table word
 *           and calls it through jalr, so the four register arguments this
 *           body never reads arrive in $a0-$a3 and the one value it uses is
 *           the fifth argument, taken from the caller's stack slot 0x10 and
 *           sign-extended as a halfword here. No instruction word of this
 *           payload holds 0x801FD748, so only that table reaches it. The body
 *           passes the work cursor coordinate pair, words 0x34 and 0x38 already
 *           in signed 16.16 form, to the shared coordinate-pair to depth helper
 *           func_8015477C and sign-extends the answer to a halfword. When that
 *           answer is below the cursor depth halfword 0x3E less 0x240, it
 *           advances the depth halfword by work word 0x14 shifted right eight
 *           bits, adds the caller's sign-extended halfword to work word 0x14
 *           and returns 1. Otherwise it calls the helper a second time on the
 *           same pair, stores the answer plus the 0x240 bias into the depth
 *           halfword, clears the handler index byte 0x04 so the next dispatch
 *           takes table word 0 and returns 0. The dispatcher narrows the answer
 *           to a signed byte, so only 0 and 1 travel back. Work word 0x14 keeps
 *           its address-anchored role: the same payload's func_801FD568 adds
 *           the caller's fifth argument to it and folds its high bits into the
 *           same depth halfword. The 0x240 bias, the direction the return value
 *           selects and the reason the index byte is cleared on the snap arm are
 *           not established, so the name and this block describe the effect only.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the two arms return constants, so the
 * compiler uses the hard return register for each arm's value instead of a join
 * pseudo, which is what puts `li v0,1` at the head of the taken arm and
 * `move v0,zero` in the untaken one. The taken arm is laid out after the
 * untaken one and falls through to the shared epilogue, so reorg copies its
 * leading constant into the `bnez` delay slot. The pointer cell is re-read
 * after each helper call and once more after the second one, because the helper
 * may rewrite it, and the untaken arm keeps both stores bound to the single
 * re-read in $a1. The second helper call re-reads coordinate words 0x34 and
 * 0x38 rather than reusing the first call's arguments.
 */
s32 advanceCursorDepthOrResnapAndClearHandlerIndexScenarioScena0600_801FD748(
    s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 delta) {
  s32 depth;

  depth = (s16)func_8015477C(*(s32*)(D_1F800044 + 0x34),
                             *(s32*)(D_1F800044 + 0x38));

  if (depth < *(s16*)(D_1F800044 + 0x3E) - 0x240) {
    *(s16*)(D_1F800044 + 0x3E) =
        *(s16*)(D_1F800044 + 0x3E) + (*(s32*)(D_1F800044 + 0x14) >> 8);
    *(s32*)(D_1F800044 + 0x14) = *(s32*)(D_1F800044 + 0x14) + (s16)delta;
    return 1;
  } else {
    *(s16*)(D_1F800044 + 0x3E) =
        func_8015477C(*(s32*)(D_1F800044 + 0x34),
                      *(s32*)(D_1F800044 + 0x38)) +
        0x240;
    D_1F800044[4] = 0;
    return 0;
  }
}
