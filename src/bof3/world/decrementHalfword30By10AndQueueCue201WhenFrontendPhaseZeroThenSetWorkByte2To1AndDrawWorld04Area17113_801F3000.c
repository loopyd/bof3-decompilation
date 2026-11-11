#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. The original
 * re-reads the cell on either side of the cue call, so it is named through this
 * target's map instead of a fixed-address accessor: the fixed-address spelling
 * keeps the cell address live across the call in a saved register, which this
 * body does not have, while the named cell is rematerialised. */
extern u8* D_1F800044;

/* Front-end phase byte of the shared main executable, outside this overlay's
 * payload; the shared symbol map and this target's generated undefined-syms map
 * both bind it, so no row is added here. The area170 lift of bytes this body
 * matches, measured exact at the time this seed was written, reads it as the
 * front-end phase that gates the 0x201 cue, while the plchar lifts leave its
 * role open, so the name stays address-anchored. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Sound-cue dispatcher of the shared main executable, outside this overlay's
 * payload and reached only through this body's cue gate; other `asm` spans of
 * this target still reference it, so the generated undefined-funcs map keeps its
 * row and no map entry is added. */
extern void func_8015DF18(u32 cue);

/* Draw routine of this overlay at 0x801F3070, still an `asm` span of this target
 * at the time of this lift: it re-reads the same work record, projects its route
 * halfwords into the scratchpad pair and builds its sprite primitive, and a
 * non-zero argument also runs its extra stage. */
void func_801F3070(s32 arg0);

/* @source 0x801F3000
 * @behavior Byte-2 state handler, entry 4 of the five-word handler run based at
 *           0x801F3F5C, whose words are 0x801F2CA0, 0x801F2E1C, 0x801F2F00,
 *           0x801F2F5C and this address, and which the overlay dispatcher at
 *           0x801F2C5C indexes by the state byte at offset 0x02 of the work
 *           record published at the scratchpad pointer cell 0x1F800044, so no
 *           jal names this body. It subtracts 0x10 from that record's unsigned
 *           halfword at offset 0x30, stores the truncated halfword, and tests
 *           the stored value read back as a signed halfword: while that value
 *           stays above zero the handler only draws. Once it reaches zero or
 *           below, the front-end phase byte D_80143BB0 gates sound cue 0x201
 *           through func_8015DF18, which fires only while that byte reads zero,
 *           and the record state byte at offset 0x02 is set to 1 so the next
 *           dispatch takes entry 1 at 0x801F2E1C, then an `asm` span of this
 *           target. The draw routine at 0x801F3070 runs on every call, including
 *           the frame that expires the countdown, with argument 0, its base-only
 *           stage. The pointer cell 0x1F800044 is loaded before the frame is
 *           created and re-read after the cue gate, both times as `lui` plus the
 *           offset load into $v1. Takes no arguments and returns nothing; the
 *           0x18-byte frame only holds $ra at 0x10 across the two calls, the
 *           slots after the halfword load and after each branch are nops, and
 *           the draw call's delay slot zeroes $a0.
 *
 * Name evidence: the byte-equal body in emi/world04/area170/13 at 0x801F4080 was
 * lifted exact as
 * decrementHalfword30By10AndQueueCue201WhenFrontendPhaseZeroThenSetWorkByte2To1AndDrawWorld04Area17013_801F4080,
 * which fixes each clause: the 0x30 halfword is a countdown stepped by 0x10, the
 * gate queues cue 0x201 only while D_80143BB0 is zero, and the state byte is then
 * set to 1. This span keeps that name and adds this target's word-4 payload
 * address as its suffix.
 *
 * Shape evidence: `cmp` of this 0x70-byte span with the 0x70-byte span at
 * 0x801F4080 in emi/world04/area170/13 differs in exactly two words, the tail jal
 * at +0x58 and its delay slot at +0x5C: each jal names its own payload's draw
 * routine, and this one passes argument 0 where that one passes nothing. The
 * spans are therefore relocation-different and this body is not a clone of a
 * lifted sibling, so only the statement order, operand forms, offset spelling and
 * declaration style carry over, with this span's own callee and call argument
 * taken from the original bytes. This target's exact neighbour at 0x801F2F00 is
 * the run's word 2 and fixes the unsigned halfword read-modify-write, the signed
 * halfword test and the 0x18-byte frame with $ra at 0x10; it has no call between
 * two cell accesses, so its fixed-address accessor spelling is not reusable here:
 * measured with that spelling this body held the cell address in $s0 across the
 * cue call and ran three instructions long (20 of 28 instructions, 64.52%),
 * while the named cell of the area170 body rematerialises the address as `lui`
 * plus the offset load, which is what this span shows on both accesses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By10AndQueueCue201WhenFrontendPhaseZeroThenSetWorkByte2To1AndDrawWorld04Area17113_801F3000(
    void) {
  u8* work;
  u16 half;

  work = D_1F800044;
  half = *(u16*)(work + 0x30) - 0x10;
  *(u16*)(work + 0x30) = half;
  if ((s16)half <= 0) {
    if (D_80143BB0 == 0) {
      func_8015DF18(0x201);
    }
    work = D_1F800044;
    work[2] = 1;
  }
  func_801F3070(0);
}
