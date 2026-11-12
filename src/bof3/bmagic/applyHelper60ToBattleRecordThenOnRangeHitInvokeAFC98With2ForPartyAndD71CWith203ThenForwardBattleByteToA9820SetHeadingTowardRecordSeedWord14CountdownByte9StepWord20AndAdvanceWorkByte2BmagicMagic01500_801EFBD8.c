#include "bof3/bof3.h"

/* Battle record pointer cell in main RAM, outside this overlay image. The
 * original keeps the cell address in $s0 across the first two helper calls, so
 * this body holds a local pointer to the cell and re-reads the word before each
 * call instead of writing the cell twice, which is the form the shape sibling
 * applyHelper60ToBattleRecordThenWhenRecordWithinC000SetHeadingTowardRecord
 * SeedWord14CountdownByte9StepWord20AndAdvanceWorkByte2BmagicMagic01500
 * _801F0194 took. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle work pointer in main RAM, outside this overlay image. Splat
 * binds the address through the encoded symbol name, so no target-map row is
 * needed. This body reads only its words 0x34 and 0x38, the aimed-at position
 * the heading is taken toward. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before each store group. A store through the loaded pointer
 * may alias the cell, so the body re-reads it for the heading vector and again
 * for the seed words and the byte 2 advance. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Acting battler byte in main RAM, outside this overlay image, tested against
 * the unsigned bound 3 before the 0x800AFC98 call. Splat's generated
 * undefined-syms map already binds the address, so the name needs no
 * target-map row. The sibling lift of this target advanceWorkByte2ByPartyOr
 * EnemyStepThenCopyRecordByte27SeedByte2BAndCopyRecordTripletBmagicMagic01500
 * _801EFAE4 treats the same byte below three as a party member. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Battle byte in main RAM, outside this overlay image, forwarded to the helper
 * at 0x800A9820. Splat's generated undefined-syms map already binds the
 * address, so the name needs no target-map row. The sibling lift
 * callAF4A0With2000IncrementWorkByte9SubtractC0FromHalfword3EThenOnCount20
 * ClearByte48SeedWorkWordsAndMotionPairWhenWorkByteBZeroForwardBattleByte
 * InvokeAFC98AndD71CWith203AdvanceWorkByte2BmagicMagic01500_801F0770 forwards
 * the same cell to the same helper. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Record helper of the main executable at 0x800AF2B4, not lifted, so its effect
 * stays unestablished. The original passes a record base and a constant; this
 * step passes the battle record and 0x60, while the counterpart steps of this
 * overlay pass 0x1800, 0x2000 and 0x3800. It returns nothing. */
extern void func_800AF2B4(u8 *record, s32 value); /* @source 0x800AF2B4 */

/* Record range test of the main executable at 0x800AF66C. The battle lift
 * testWorkRangeAgainstRecord owns the body: it advances the scratch work object
 * 0x34 and 0x38 axes by half the passed value, reduces each by the record
 * matching origin and returns 1 only when the value reaches both bounds. The
 * constant is unsigned here, which is why the original materializes 0xC000 with
 * a single ori instead of a lui and ori pair. */
extern u32 func_800AF66C(u8 *record, u32 value); /* @source 0x800AF66C */

/* Shared main-executable helper, outside every payload. Its effect is not
 * established, so the raw address name stays and no map row is needed. This
 * body passes 2 and 0, the constants its sibling lifts pass, and reaches it
 * only while the acting battler byte stays below three. */
extern void func_800AFC98(s32 arg0, s32 arg1); /* @source 0x800AFC98 */

/* Shared main-executable helper, outside every payload. Its effect is not
 * established, so the raw address name stays and no map row is needed. This
 * body passes the selector 0x203. */
extern void func_8015D71C(s32 selector); /* @source 0x8015D71C */

/* Shared main-executable helper, outside every payload. Its effect is not
 * established, so the raw address name stays and no map row is needed. Its one
 * argument is forwarded from the byte at 0x80146384. */
extern void func_800A9820(u8 index); /* @source 0x800A9820 */

/* Angle helper of the main executable at 0x8017A138: it turns a position
 * difference into a 12-bit heading. The sibling lifts of this tree call it with
 * the same word pairs taken from two records and store the result as the work
 * object direction word 0xC. */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* @source 0x801EFBD8
 * @behavior Entry 14 of the overlay header handler table that starts at
 *           0x801EEC04, the word the table stores at 0x801EEC3C and index 2 of
 *           the eight-word sub-run at 0x801EEC34 that dispatchWorkByte2Handler
 *           ThenInvokeSharedServiceWhenWorkBytes0And2SetBmagicMagic01500
 *           _801EFA28 copies into its frame and selects with unsigned work byte
 *           2 of the scratchpad work object held in the pointer cell 0x1F800044.
 *           The payload holds that one table word and no jal or j word to this
 *           address, so the call site is that indirect dispatch. It passes the
 *           battle record read through the pointer cell 0x8014638C with the
 *           constant 0x60 to the main-executable record helper 0x800AF2B4,
 *           then re-reads that cell and passes the same record with 0xC000 to
 *           the range test 0x800AF66C, returning while the test reports no hit.
 *           On a hit it runs the battler class block: while the acting battler
 *           byte 0x80146374 stays below three it invokes 0x800AFC98 with 2 and
 *           0, and it then invokes 0x8015D71C with the selector 0x203 and
 *           forwards the battle byte 0x80146384 to 0x800A9820. It then computes
 *           the heading of the battle record read through 0x801EB4E0 relative
 *           to the scratchpad work object published at 0x1F800044 - word 0x34
 *           difference as x and word 0x38 difference as y - through the angle
 *           helper 0x8017A138, stores that result as work word 0xC, and seeds
 *           work word 0x14 = 0x40, work countdown byte 9 = 0x10 and work step
 *           word 0x20 = -8, the value the sibling steps add to word 0x14 on
 *           each frame. It then re-reads the work cell and advances work byte
 *           2, the handler index, at byte width. The record cell address stays
 *           in $s0 across the first two calls, the first cell load follows the
 *           $ra store, and the 0x18-byte frame keeps $s0 at 0x10 and $ra at
 *           0x14. The 108-byte tail from 0x801EFC44 with its 20-byte epilogue is
 *           byte-identical to the span at 0x801F01C8 of
 *           applyHelper60ToBattleRecordThenWhenRecordWithinC000SetHeading
 *           TowardRecordSeedWord14CountdownByte9StepWord20AndAdvanceWorkByte2
 *           BmagicMagic01500_801F0194 in this same payload, which a cmp over the
 *           128 bytes proved; that exact body supplied the statement order,
 *           operand forms and offset spelling, and the battler class block that
 *           precedes the tail comes from this span alone. Nothing is returned
 *           and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyHelper60ToBattleRecordThenOnRangeHitInvokeAFC98With2ForPartyAndD71CWith203ThenForwardBattleByteToA9820SetHeadingTowardRecordSeedWord14CountdownByte9StepWord20AndAdvanceWorkByte2BmagicMagic01500_801EFBD8(
    void) {
  u8 **recordCell;
  u8 *record;
  u8 *target;
  u8 *work;
  s32 heading;
  u8 *step;

  recordCell = &D_8014638C;
  func_800AF2B4(*recordCell, 0x60);
  if (func_800AF66C(*recordCell, 0xC000)) {
    if (D_80146374 < 3) {
      func_800AFC98(2, 0);
    }
    func_8015D71C(0x203);
    func_800A9820(D_80146384);
    record = D_801EB4E0;
    target = D_1F800044;
    heading = func_8017A138(*(s32 *)(record + 0x34) - *(s32 *)(target + 0x34),
                            *(s32 *)(record + 0x38) - *(s32 *)(target + 0x38));
    work = D_1F800044;
    *(s32 *)(work + 0xC) = heading;
    *(s32 *)(work + 0x14) = 0x40;
    work[9] = 0x10;
    step = D_1F800044;
    *(s32 *)(work + 0x20) = -8;
    step[2]++;
  }
}
