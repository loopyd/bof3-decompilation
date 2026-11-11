#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. The original
 * re-reads the cell once on each side of the cue call, so it is named instead
 * of taken through a fixed-address accessor: the named cell is rematerialised
 * as `lui` plus the offset load, while the fixed-address spelling keeps it live
 * in a saved register across the call. */
extern u8* D_1F800044;

/* Front-end phase byte of the main executable, outside this overlay's payload.
 * The cue below only fires while that phase holds zero; the exact world04
 * area165 and area170 lifts of this body read the same gate. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Sound-cue dispatcher of the main executable, outside this overlay's payload. */
void func_8015DF18(u16 cue);

/* @source 0x801F3864
 * @behavior Byte-2 state handler 4 of the local handler table based at
 *           0x801F3924, the word that table stores at 0x801F3934, for the work
 *           record published at the scratchpad pointer cell 0x1F800044.
 *           Subtracts 0x20 from the record halfword at 0x30, stores the
 *           truncated halfword and tests the stored value as a signed halfword:
 *           while that value stays above zero the handler returns without
 *           touching anything else. Once the countdown reaches zero or below,
 *           the front-end phase byte D_80143BB0 gates cue 0x201 through
 *           func_8015DF18, which fires only while that byte is zero, and the
 *           record's state byte at 0x02 is set to 1 so the next dispatch takes
 *           entry 1 at 0x801F35E0 of the same table. The pointer cell is loaded
 *           before the frame is created and re-read after the cue gate, both
 *           times into $v1. Takes no arguments and returns nothing; the
 *           0x18-byte frame only keeps $ra at 0x10, and the slots after the
 *           halfword load and after each branch stay nops.
 *
 * Name evidence: the same three-statement body lifted exactly in
 * emi/world04/area170/13 at 0x801F4080 is published as
 * decrementHalfword30By10AndQueueCue201WhenFrontendPhaseZeroThenSetWorkByte2To1AndDraw,
 * which fixes the role of the 0x30 halfword countdown, the 0x201 cue, the
 * D_80143BB0 phase gate and the state-byte store to 1; only the subtracted
 * amount (0x20 here, from this span's own `addiu $v0, $v0, -0x20`) and the
 * absent draw tail differ, so the name keeps that spelling and drops the
 * `...AndDraw` clause.
 *
 * Shape evidence: the `asm` span at 0x801F3098 of this same target carries the
 * identical 22 leading words and adds a `jal 0x801F3108` plus its delay slot
 * before the same epilogue; it is the same body plus one call, a same-target
 * twin, so it is shape evidence and never clone eligibility. A byte scan of
 * every extracted payload for this 0x68-byte span finds only this span itself.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By20AndQueueCue201WhenFrontendPhaseZeroThenSetWorkByte2To1World04Area15713_801F3864(
    void) {
  u8* work;
  u16 half;

  work = D_1F800044;
  half = *(u16*)(work + 0x30) - 0x20;
  *(u16*)(work + 0x30) = half;
  if ((s16)half <= 0) {
    if (D_80143BB0 == 0) {
      func_8015DF18(0x201);
    }
    work = D_1F800044;
    work[2] = 1;
  }
}
