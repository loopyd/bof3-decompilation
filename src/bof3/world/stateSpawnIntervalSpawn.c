#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row. The original re-materialises the cell with a `lui 0x1F80` plus a load
 * at displacement 0x44 for each of the four access groups in the body below, so
 * the cell is read through the map name rather than a constant-address accessor,
 * whose shared base would stay live across the two calls in a saved register and
 * widen the frame. */
extern u8* scratchWorkPointerAt1f800044; /* @source 0x1F800044 @kind data */

/* Work-table reset at 0x801F40A4: clears the lead byte of each of the 0x18
 * entries of the 0x38-byte-stride work table at 0x800E4800. That lift is exact
 * and records this span as its only caller inside the payload. */
extern void clearWorkTableAt800E4800World02Area09813_801F40A4(void); /* @source 0x801F40A4 @kind function */

/* Cue submission at 0x8015DF18: takes the cue id in $a0 and returns nothing. Its
 * body lies outside this payload, so the argument role comes from the exact
 * lifts of this overlay engine that queue cue ids through it. */
void func_8015DF18(u16 cue);

/* @source 0x801F3540
 * @behavior Overlay progress step handler, entry 0 of the twelve-word handler
 *           row based at 0x801F5F7C. It first calls the work-table reset
 *           0x801F40A4, which clears the lead byte of each of the 0x18 records
 *           of the 0x38-byte-stride table at 0x800E4800. It then reads the work
 *           record through the scratchpad pointer cell 0x1F800044 and stores
 *           0xFF into that record's byte at +0x09, zero into the byte at +0x0A
 *           and 0x10 into the byte at +0x06, each store going through its own
 *           fresh read of the cell. A fourth read of the cell serves the state
 *           byte at +0x01, which it advances by one so the record passes to entry
 *           1 of the row, and the body then queues cue 0x204 through 0x8015DF18.
 *           It takes no arguments, returns nothing, and its 0x18-byte frame
 *           holds only $ra at 0x10 across the two calls.
 *
 *           Dispatch evidence: the payload words 0x801F5F7C through 0x801F5FA8
 *           are the twelve entries 0x801F3540, 0x801F35B0, 0x801F36A0,
 *           0x801F3814, 0x801F3878, 0x801F3910, 0x801F3994, 0x801F3AA8,
 *           0x801F3B5C, 0x801F3BD4, 0x801F3CDC and 0x801F3D98, and the exact
 *           dispatcher dispatchStateByte1Table5f7cWorld02Area09813_801F34FC
 *           scales the work record state byte at +0x01 by four onto that base and
 *           jalrs the selected entry, so this span is the state-0 progress
 *           handler of that row and its caller stays outside the payload. No jal
 *           immediate in this payload names 0x801F3540, so the absence of
 *           arguments and the void return are structural rather than declared.
 *
 *           Naming evidence: the three bytes this body arms are consumed by entry
 *           1 of the same row, the exact
 *           spawnRecordOnShrinkingIntervalRunDrawPassAndArmMotionOnSharedStateMatchWorld02Area09813_801F35B0,
 *           whose own prose records this span storing 0x10 into +0x06 and zero
 *           into +0x0A; that entry compares +0x0A with +0x06, advances +0x0A while
 *           the two differ, and on the frame they meet calls the record spawn
 *           0x801F40D0 and subtracts 2 from +0x06, so +0x06 is the frame interval
 *           between spawns and +0x0A the progress counter compared against it.
 *           Entry 2, 0x801F36A0, tests the +0x09 byte this body stores 0xFF into
 *           against 0x14 and increments it only while it stays below 0x15, so
 *           0xFF sits above both that test and that bound until entry 1 clears the
 *           byte on the spawn frame; the exact lifts of this target read the same
 *           offsets for those roles. The cue id 0x204 is the next value above the
 *           0x200 the neighbouring row queues at 0x801F3E64, and 0x8015DF18 is
 *           the cue submission the exact lifts of this engine queue ids through.
 *           The two-argument-free (`void`) call and the block order come from this
 *           span's own bytes.
 *
 *           Shape evidence: the 112-byte span is not byte-identical to any span
 *           of the corpus - a scan of every payload under out/binaries placed the
 *           112-byte window only at offset 0x940 of this payload, which is this
 *           span - so no clone applied. The nearest same-target exact lift,
 *           0x801F3E64, shares the frame, the post-call cell read and the
 *           cue-queue tail but calls a different reset (0x801F4D44), stores 0x40
 *           and zero into two halfwords rather than four bytes, and queues 0x200,
 *           so it is shape evidence for the cell read and the tail only and the
 *           statement order of the four byte stores comes from this span's own
 *           bytes.
 *
 *           Representation: each of the four cell groups is its own statement
 *           reading scratchWorkPointerAt1f800044, which is why the original emits
 *           a lui/lw pair per group rather than keeping one base live; the three
 *           arming stores are u8 literals and the state advance is written as a
 *           read-modify-write through the fourth read, which is the lbu/addiu/sb
 *           triple the original carries with the sb placed in the call's delay
 *           slot; the cue argument is declared u16 as in the exact lifts that
 *           queue cue ids through 0x8015DF18. No alternative spelling was
 *           measured before the first gate: this body was written directly from
 *           the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSpawnIntervalSpawn(
    void) {
  clearWorkTableAt800E4800World02Area09813_801F40A4();
  scratchWorkPointerAt1f800044[9] = 0xFF;
  scratchWorkPointerAt1f800044[0xA] = 0;
  scratchWorkPointerAt1f800044[6] = 0x10;
  scratchWorkPointerAt1f800044[1] = scratchWorkPointerAt1f800044[1] + 1;
  func_8015DF18(0x204);
}
