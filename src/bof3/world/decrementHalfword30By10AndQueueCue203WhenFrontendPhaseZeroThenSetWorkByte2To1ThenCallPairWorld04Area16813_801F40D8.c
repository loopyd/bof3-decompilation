#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. This target's
 * map binds the cell at 0x1F800044, and the original re-reads the cell on
 * either side of the sound-cue gate, so the named cell is rematerialised rather
 * than kept live across the call.
 * @source 0x1F800044 @kind data
 */
extern u8* area16813WorkCursor_1F800044;

/* Front-end phase byte of the shared main executable, outside this payload;
 * Splat binds it through the shared symbol map. The world04 sibling handlers
 * read it as the front-end phase that gates a cue while it is zero, and the
 * plchar lifts leave its role open, so the symbol stays address-anchored.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Sound-cue dispatcher of the shared main executable, outside this payload; the
 * exact sibling handler 0x801F38A4 passes it a cue id and discards any result.
 * @source 0x8015DF18 @kind function
 */
extern void func_8015DF18(u16 cue);

/* Mode helper of this payload, still an `asm` span at 0x801F4158: it indexes
 * the per-mode byte table at 0x801F4BB4 by its first argument, re-reads the same
 * work record and writes scratchpad and GTE state, and answers nothing. The
 * exact neighbours 0x801F3DB0 and 0x801F3F54 call it with this same pair of
 * literals.
 * @source 0x801F4158 @kind function
 */
extern void func_801F4158(s32 index, s8 mode);

/* @source 0x801F40D8
 * @behavior Byte-2 state handler reached through the handler run based at
 *           0x801F4BA0 of this payload: the body address appears once in the
 *           payload bytes, at word 0x801F4BB0, which is entry 4 of that run, so
 *           no jal in this payload names it, it takes no arguments and it
 *           answers no value. The body reads the work record through the
 *           scratchpad pointer cell 0x1F800044, subtracts 0x10 from the
 *           record halfword at offset 0x30, stores the truncated halfword and
 *           tests the stored value as a signed halfword: while that value stays
 *           above zero the handler only runs the tail pair below. Once the
 *           countdown reaches zero or below, the shared front-end phase byte
 *           D_80143BB0 gates the sound cue 0x203 through func_8015DF18, which
 *           fires only while that byte reads zero, and the record's state byte
 *           at offset 0x02 is set to 1 so the next dispatch takes entry 1 of
 *           the same run. The pointer cell is loaded before the frame is
 *           created and re-read after the cue gate, both times as `lui` plus the
 *           offset load into $v1. The local helper 0x801F4158 then runs on every
 *           call, first as (0, 1) and then as (1, 2); its first argument is
 *           loaded as zero into the delay slot of the countdown branch, and as
 *           zero again after the cue gate, because the cue id took $a0. The
 *           0x18-byte frame only keeps $ra at 0x10, and the slots after the
 *           halfword load and after each of the two inner branches stay nops.
 *           The roles of the 0x30 halfword, of the 0x2 state byte and of the
 *           requested cue stay address-anchored.
 *
 * Name evidence: the exact sibling of this target at 0x801F38A4 is the same
 * countdown handler of the neighbouring run and differs at the sites its own
 * bytes fix - it branches to the same countdown test and cue gate, requests cue
 * 0x201 instead of 0x203, and ends in the single helper 0x801F3914 where this
 * body ends in the (0, 1) and (1, 2) pair, which is byte-for-byte the tail of
 * the exact seed 0x801F3DB0 of the same handler run. A payload search over all
 * 396 overlay binaries found no second copy of this 0x80-byte span, and the
 * absolute jal immediate 0x801F4158 is payload-local, so the span has no
 * byte-identical sibling to clone and the two neighbours are shape evidence.
 *
 * Shape evidence: the exact sibling 0x801F38A4 fixes the named-cell accessor,
 * the halfword read-modify-write, the sign test on the stored halfword and the
 * cue gate; the exact neighbour 0x801F3DB0 fixes the `func_801F4158(0, 1)` and
 * `func_801F4158(1, 2)` call pair and its argument placement.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By10AndQueueCue203WhenFrontendPhaseZeroThenSetWorkByte2To1ThenCallPairWorld04Area16813_801F40D8(
    void) {
  u8* work;
  u16 half;

  work = area16813WorkCursor_1F800044;
  half = *(u16*)(work + 0x30) - 0x10;
  *(u16*)(work + 0x30) = half;
  if ((s16)half <= 0) {
    if (D_80143BB0 == 0) {
      func_8015DF18(0x203);
    }
    work = area16813WorkCursor_1F800044;
    work[2] = 1;
  }
  func_801F4158(0, 1);
  func_801F4158(1, 2);
}
