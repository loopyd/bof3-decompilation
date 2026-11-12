#include "bof3/bof3.h"

/* Battle record pointer cell in main RAM, outside this overlay image. The
 * original keeps the cell address in $s0 across both helper calls, so this body
 * holds a local pointer to the cell and re-reads the word before each call
 * instead of writing the cell twice, which is the form the shape sibling
 * applyHelper60ToBattleRecordThenAdvanceWorkByte2WhenWorkWithin20000OfRecord
 * BmagicMagic02003_801EFA70 took. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle work pointer in main RAM, outside this overlay image. Splat
 * binds the address through the encoded symbol name, so no target-map row is
 * needed. The sibling lifts of this target read the same record and copy or
 * guard on its words 0x34 and 0x38; this body only reads those two words as the
 * aimed-at position. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before each store group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Record helper of the main executable at 0x800AF2B4, not lifted, so its effect
 * stays unestablished. The original passes a record base and a constant; this
 * step passes the battle record and 0x60, while the counterpart steps pass
 * 0x1800, 0x2000 and 0x3800. It returns nothing. */
extern void func_800AF2B4(u8 *record, s32 value); /* @source 0x800AF2B4 */

/* Record range test of the main executable at 0x800AF66C. The battle lift
 * testWorkRangeAgainstRecord owns the body: it advances the scratch work
 * object 0x34 and 0x38 axes by half the passed value, reduces each by the
 * record matching origin and returns 1 only when the value reaches both bounds.
 * The constant is unsigned here, which is why the original materializes 0xC000
 * with a single ori instead of a lui and ori pair. */
extern u32 func_800AF66C(u8 *record, u32 value); /* @source 0x800AF66C */

/* Angle helper of the main executable at 0x8017A138: it turns a position
 * difference into a 12-bit heading. The sibling lifts of this tree call it with
 * the same word pairs taken from two records and store the result as the work
 * object direction word 0xC. */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* @source 0x801F0194
 * @behavior Entry 22 of this overlay header handler code-pointer run that starts
 *           at 0x801EEC04: the run stores this address at 0x801EEC5C, between
 *           func_801F014C and the entry 23 handler at 0x801F0248, and the
 *           payload holds that one address word and no jal or j word to this
 *           address, so the call site is the indirect dispatch through the
 *           copied run. It passes the battle record read through the pointer
 *           cell 0x8014638C with the constant 0x60 to the main-executable
 *           record helper 0x800AF2B4, then re-reads that cell and passes the
 *           same record with 0xC000 to the range test 0x800AF66C, returning
 *           while the test reports no hit. On a hit it computes the heading of
 *           the battle record read through 0x801EB4E0 relative to the
 *           scratchpad work object published at 0x1F800044 - word 0x34
 *           difference as x and word 0x38 difference as y - through the angle
 *           helper 0x8017A138, stores that result as work word 0xC, and seeds
 *           work word 0x14 = 0x40, work countdown byte 9 = 0x10 and work step
 *           word 0x20 = -8, the value the sibling steps add to word 0x14 on
 *           each frame. It then re-reads the work cell and advances work byte
 *           2, the handler index, at byte width. The record cell address stays
 *           in $s0 across both calls, the first cell load follows the $ra
 *           store, and the 0x18-byte frame keeps $s0 at 0x10 and $ra at 0x14.
 *           The 108-byte tail from 0x801F01C8 and the 20-byte epilogue are
 *           byte-identical to the span at 0x801EFC44 of func_801EFBD8 in this
 *           same payload (cmp over the 128 bytes), whose disassembly supplied
 *           the statement order, operand forms and offset spelling; that
 *           sibling head differs only in the branch immediate because it runs a
 *           battler-class block before the same tail. The lifts
 *           applyHelper60ToBattleRecordThenAdvanceWorkByte2WhenWorkWithin20000
 *           OfRecordBmagicMagic02003_801EFA70 and drawBattlerRecordPointSnap
 *           PositionWhenSlotUnavailableAndOnRangeHitRaiseFlag2000ForwardSlot
 *           SetHeadingTowardBattleRecordAndAdvanceWorkByte2BmagicMagic01200
 *           _801EF188 of other targets were exact when this body was written and
 *           supplied the call sequence and the heading-toward-record statement
 *           set. Nothing is returned and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyHelper60ToBattleRecordThenWhenRecordWithinC000SetHeadingTowardRecordSeedWord14CountdownByte9StepWord20AndAdvanceWorkByte2BmagicMagic01500_801F0194(
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
