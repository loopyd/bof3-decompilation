#include "bof3/bof3.h"

/* Shared battle record pointer cell in main RAM, outside this overlay's image.
 * Splat's generated undefined-syms map already binds the address, so the name
 * needs no target-map row. This body loads the record twice, so it keeps the
 * cell address in a local: writing both accesses on the named symbol instead
 * folds the address into each load and loses the callee-saved base the original
 * builds ahead of the return-address store. Each load still re-reads the cell,
 * because the helper may republish the record. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle work record pointer cell in main RAM, read for the second
 * helper call only. Splat's generated undefined-syms map already binds the
 * address, so the name needs no target-map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. This target's map binds no name at the
 * address, so the D_ spelling auto-binds with no map row, as it does for the
 * scratchpad lifts of this target that already measure exact. A store through
 * the loaded pointer may alias the cell, so the body re-reads it before each
 * store group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared main-executable helper, outside every payload. Its role is not
 * established, so the raw address name stays and no map row is needed. */
extern void func_800AF4A0(u8 *record, s32 value, s32 extra); /* @source 0x800AF4A0 */

/* @source 0x801F0570
 * @behavior Entry 26 of the overlay handler table at 0x801EEC04, the word
 *           stored at 0x801EEC6C: it first passes the shared battle record read
 *           through the pointer cell 0x8014638C together with 0x1800 and a zero
 *           third argument to helper 0x800AF4A0, then steps the scratchpad work
 *           object published at the pointer cell 0x1F800044. The step decrements
 *           countdown byte 9 and keeps the decremented value in a byte local so
 *           the truncation supplies the compared value, adds work word 0x20 to
 *           work word 0x14 while accumulating the updated word 0x14 into work
 *           halfword 0x3E, subtracts 0x800 from the 16.16 motion word 0x40 and
 *           adds 0x1000 to the motion word 0x44, and returns while the new
 *           countdown is nonzero. When the countdown reaches zero it re-reads
 *           both cells, copies words 0x34 and 0x38 of the shared battle record
 *           into the same work offsets, sets work halfword 0x3E to record
 *           halfword 0x3E plus 0xC00, passes the shared battle work record read
 *           through the pointer cell 0x801EB4E0 with 0x20000 and a zero third
 *           argument to the same helper, and then advances work byte 2, the
 *           handler index, with a byte-width read-modify-write. The work cell is
 *           re-read before the record copy group because the helper call may
 *           alias it, and the 0x18-byte frame keeps the return address at 0x14
 *           and the record-cell address in a saved register across both calls.
 *           No jal or j word in the payload targets this address; the only word
 *           naming it is the handler-table entry at 0x801EEC6C, so the call site
 *           is the indirect dispatch through that run. Takes no arguments,
 *           returns nothing and reads or writes no other state; work and record
 *           field roles remain offset-anchored.
 * @status partial
 * @match 83.05
 * @residual The first 28 instructions match. The tail keeps the record in $v0
 *           and the copy temporary in $v1, as the original does, but advances
 *           the record cursor with two extra address updates. The battle-record
 *           argument load and argument setup are scheduled differently, and the
 *           countdown branch targets the shifted epilogue (236 versus 228 bytes).
 */
/* Probe record, kept so a later rung does not repeat it. Writing both record
 * accesses on the named cell symbol folded the address into each load and lost
 * the saved-register base: 20/57 instructions. One work pointer reused by both
 * store blocks also lost that base: 27/59. A separate tail pointer restored it
 * and the first block matched at 45/59; adding the second helper call's record
 * as a tail local moved the copy pointer off the argument register and reached
 * 48/58. Declaration order of the locals (four orders), statement order of the
 * countdown against the step, an early-return form, and explicit word and
 * halfword temporaries each measured the same 48/58 first difference at +0x70. */
void callAF4A0With1800DecrementWorkByte9AddWorkWord20ToWord14AccumulateIntoHalfword3EStepMotionPairThenOnCountdownCopyRecordWordsAddC00ToHalfword3ECallAF4A0With20000AndAdvanceWorkByte2BmagicMagic01500_801F0570(void) {
  u8 **recordCell;
  u8 *work;
  u8 *copy;
  u8 *battle;
  u16 half;
  u8 *record;
  u8 count;
  s32 step;

  recordCell = &D_8014638C;
  func_800AF4A0(*recordCell, 0x1800, 0);
  work = D_1F800044;
  count = (u8)(work[9] - 1);
  work[9] = count;
  step = *(s32 *)(work + 0x14) + *(s32 *)(work + 0x20);
  *(s32 *)(work + 0x14) = step;
  *(u16 *)(work + 0x3E) += step;
  *(s32 *)(work + 0x40) -= 0x800;
  *(s32 *)(work + 0x44) += 0x1000;
  if (count == 0) {
    record = *recordCell;
    battle = D_801EB4E0;
    copy = D_1F800044;
    record += 0x34;
    *(u32 *)(copy + 0x34) = *(u32 *)record;
    record += 4;
    *(u32 *)(copy + 0x38) = *(u32 *)record;
    record += 6;
    half = *(u16 *)record + 0xC00;
    *(u16 *)(copy + 0x3E) = half;
    func_800AF4A0(battle, 0x20000, 0);
    D_1F800044[2]++;
  }
}
