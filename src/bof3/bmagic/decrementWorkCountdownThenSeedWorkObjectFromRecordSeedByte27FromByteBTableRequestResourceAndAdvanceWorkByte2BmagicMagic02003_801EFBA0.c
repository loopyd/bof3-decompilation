#include "bof3/bof3.h"

/* Battle record pointer published at 0x8014638C in main RAM, outside this
 * overlay image. Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. The exact lifts of
 * this target reach the same cell (applyHelper60ToBattleRecordThenAdvanceWork
 * Byte2WhenWorkWithin20000OfRecordBmagicMagic02003_801EFA70 passes it to the
 * record helpers), and its byte 8 and words 0x34, 0x38 and 0x3C are the fields
 * this handler copies into the work object. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell, spelled with the named cell the exact
 * lifts of this target already use. A named cell symbol lets the assembler fold
 * the constant address into each load (lui $vN,0x1F80 / lw $vN,0x44($vN)), and
 * the original re-reads the cell once per store group because a store through
 * the loaded pointer may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Overlay-local byte table at 0x801EFEC4: the nine bytes that follow the
 * three-word handler run based at 0x801EFEB8 in this payload's textbin tail.
 * The span is data, not code - no prologue and no jr $ra - and this handler
 * uses it as a table base, scaling no index. The nine entries read 0, 1, 1, 2,
 * 3, 4, 5, 5, 2. */
extern u8 D_801EFEC4[]; /* @source 0x801EFEC4 @kind data */

/* Shared main-executable helper at 0x800AF764. Five exact bmagic lifts declare
 * the same no-argument prototype and call it once per step, so its role stays
 * unestablished; this handler calls it after the record copy, with $a0 still
 * holding the record it no longer needs. */
extern void func_800AF764(void); /* @source 0x800AF764 */

/* Resource request wrapper of the main executable at 0x8014D6B8. The exact
 * bmagic lifts of this target declare the same prototype and pass a resource id
 * in the first slot. */
extern void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */

/* @source 0x801EFBA0
 * @behavior Work byte 2 step handler of the two-entry run 0x801EFAF4 stages on
 *           its own frame (0x801EFBA0 and 0x801EFD10) and selects by the work
 *           object's unsigned byte 2, so no jal in this payload names it and it
 *           runs while byte 2 still holds 0. It decrements the scratchpad work
 *           object's countdown at byte 9 (lbu / addiu -1 / sb) and tests the
 *           truncated byte it just stored (andi 0xFF of the stored register, no
 *           second load of the cell) against zero, branching to the shared
 *           epilogue at 0x801EFD00 while the countdown is still running. Only on
 *           the last tick does it run the seeding path: it reads the battle
 *           record published at 0x8014638C once, copies record byte 8 into work
 *           byte 8, re-reads the work pointer cell and copies record words 0x34,
 *           0x38 and 0x3C over the matching work words through one load, then
 *           calls the shared main-executable helper func_800AF764 with no
 *           argument setup. It seeds the display state next - byte 0x29 = 0,
 *           byte 0x25 = 0x1D, byte 0x26 = 0, byte 0x24 = 0x84, then byte 0x2B = 0
 *           and the halfword at 0x2C = 0 through one load, with the cell load of
 *           the following group taken between those two stores so the halfword
 *           lands in its load delay slot, then byte 0x28 = 0 - each group
 *           re-reading the pointer cell because a store through the pointer it
 *           supplied may alias the cell. Work byte 0x27 then takes
 *           the overlay-local byte table entry at 0x801EFEC4 indexed by work
 *           byte 0xB plus 0xB0, so it lands in 0xB0-0xB5. It re-reads the cell
 *           and sets work byte 0x2A to 1 when the unsigned value (work byte 8 -
 *           1) is below 2, and to 0 otherwise, requests the resource named by
 *           work byte 0xB through func_8014D6B8, and advances work byte 2 by one
 *           at byte width, which moves the dispatch to 0x801EFD10. It takes no
 *           arguments, returns nothing and writes no state outside the
 *           scratchpad work object. The frame is 0x18 bytes with the return
 *           address at 0x10 and no saved registers; the pointer-cell load
 *           precedes frame creation, and the store into work word 0x3C lands in
 *           the jal delay slot.
 *
 *           The clone lever is declined: the twelve-byte window carrying the
 *           table lookup (lui $at,0x801F / addu / lbu 0xFEC4) occurs exactly once
 *           under out/binaries, in this payload, and the table address is this
 *           target's own fixed address, so no byte-identical sibling exists
 *           anywhere. The data and carve levers do not apply: the span starts
 *           with a pointer-cell load, ends in jr $ra and sits between the
 *           reviewed boundaries at 0x801EFBA0 (end of 0x801EFAF4) and
 *           0x801EFD10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkCountdownThenSeedWorkObjectFromRecordSeedByte27FromByteBTableRequestResourceAndAdvanceWorkByte2BmagicMagic02003_801EFBA0(void) {
  u8 *work;
  u8 countdown;
  u8 *source;
  u8 *copyByte;
  s32 *copy;
  u8 *index;
  u8 *header;
  u8 *next;
  u8 *kind;
  u8 *pair;
  u8 *offset;
  u8 *tilt;
  u8 *tail;
  u8 *clear;
  u8 *step;

  work = D_1F800044;
  countdown = work[9] - 1;
  work[9] = countdown;
  if (countdown == 0) {
    source = D_8014638C;
    copyByte = D_1F800044;
    copyByte[8] = source[8];
    copy = (s32 *)D_1F800044;
    copy[13] = ((s32 *)source)[13];
    copy[14] = ((s32 *)source)[14];
    copy[15] = ((s32 *)source)[15];
    func_800AF764();
    index = D_1F800044;
    index[0x29] = 0;
    header = D_1F800044;
    header[0x25] = 0x1D;
    next = D_1F800044;
    next[0x26] = 0;
    kind = D_1F800044;
    kind[0x24] = 0x84;
    pair = D_1F800044;
    pair[0x2B] = 0;
    offset = D_1F800044;
    *(u16 *)(pair + 0x2C) = 0;
    offset[0x28] = 0;
    tilt = D_1F800044;
    tilt[0x27] = D_801EFEC4[tilt[0xB]] + 0xB0;
    tail = D_1F800044;
    if ((u32)(tail[8] - 1) < 2) {
      tail[0x2A] = 1;
    } else {
      tail[0x2A] = 0;
    }
    clear = D_1F800044;
    func_8014D6B8(clear[0xB]);
    step = D_1F800044;
    step[2]++;
  }
}
