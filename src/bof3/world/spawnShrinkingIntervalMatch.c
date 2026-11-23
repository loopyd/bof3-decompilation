#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row. The original re-materialises the cell with a `lui 0x1F80` plus a load
 * at displacement 0x44 for each of the four access groups below, so the cell is
 * read through the map name rather than a constant-address accessor, whose
 * shared base would stay live across the two calls in a saved register and
 * widen the frame. */
extern u8* scratchWorkPointerAt1f800044; /* @source 0x1F800044 @kind data */

/* Record claim and seed at 0x801F40D0: it walks the lead bytes of the records of
 * the shared table at 0x800E4800, takes the first free one and seeds it from
 * rand(). Its body is still an asm boundary in this payload, so it keeps its
 * address anchor. */
void func_801F40D0(void);

/* Effect-table draw pass at 0x801F42E8: it sets the draw mode and the texture
 * page for the shared 0x800E4800 record table and the 0x800E48C0 entry table.
 * Its body is still an asm boundary in this payload, so it keeps its address
 * anchor. */
void func_801F42E8(void);

/* Shared main-executable cells the trigger tests. The first is the state byte the
 * exact plchar lifts of this corpus write 2 into as a busy mark and read as a
 * done-report suppressor, the second is the shared status halfword the exact
 * lifts of this target store 0x2F and 0xFFFF into, and the third's role is not
 * established anywhere, so all three keep their address anchor. */
extern u8 D_80143BB0;
extern u16 D_801490A8;
extern u8 D_80146875;

/* @source 0x801F35B0
 * @behavior Overlay progress step handler, entry 1 of the twelve-word handler
 *           row based at 0x801F5F7C. It reads the work record through the
 *           scratchpad pointer cell 0x1F800044 and compares that record's byte
 *           at +0x0A with its byte at +0x06: while the two differ it stores the
 *           byte at +0x0A advanced by one and leaves through the tail below. On
 *           the frame the two are equal it calls the record spawn 0x801F40D0,
 *           re-reads the cell, subtracts 2 from the byte at +0x06 when that byte
 *           is nonzero and clears +0x0A, so the interval between spawns shrinks
 *           by two frames per spawn and reaches zero. Either path then runs the
 *           draw pass 0x801F42E8 and tests three shared main-executable cells:
 *           while the state byte 0x80143BB0 reads 2, or the status halfword
 *           0x801490A8 does not read 0x12, or the byte 0x80146875 does not read
 *           0xC, the handler returns. When all three hold it clears the countdown
 *           byte at +0x09, stores 0xA000 into the motion word at +0x0C and
 *           0xB000 into the motion word at +0x10, clears the offset words at
 *           +0x18 and +0x1C and advances the state byte at +0x01 through a fresh
 *           read of the cell, which hands the record to entry 2 of the row. It
 *           takes no arguments, returns nothing and its 0x18-byte frame holds
 *           only $ra at 0x10 across the two calls.
 *
 *           Dispatch evidence: the payload words 0x801F5F7C through 0x801F5FA8
 *           are the twelve entries 0x801F3540, 0x801F35B0, 0x801F36A0,
 *           0x801F3814, 0x801F3878, 0x801F3910, 0x801F3994, 0x801F3AA8,
 *           0x801F3B5C, 0x801F3BD4, 0x801F3CDC and 0x801F3D98, and the exact
 *           dispatcher dispatchStateByte1Table5f7cWorld02Area09813_801F34FC
 *           scales the work record state byte at +0x01 by four onto that base and
 *           jalrs the selected entry, so this span is entry 1 of that row and its
 *           caller stays outside the payload. No jal immediate in this payload
 *           names 0x801F35B0, so the absence of arguments and the void return are
 *           structural rather than declared.
 *
 *           Naming evidence: entry 0 of the same row, 0x801F3540, arms exactly the
 *           two bytes this span consumes, storing 0x10 into +0x06 and zero into
 *           +0x0A before advancing the state byte, which fixes +0x06 as the frame
 *           interval that this span counts down and +0x0A as the progress counter
 *           it advances against it; the -2 store on the interval is what makes the
 *           interval shrink. Entry 2, 0x801F36A0, consumes what this span arms on
 *           the trigger frame: it counts +0x09 up to 0x14 and integrates the two
 *           motion words at +0x0C and +0x10 into the offset words at +0x18 and
 *           +0x1C, so +0x0C and +0x10 are motion words and +0x18/+0x1C the offsets
 *           they drive, which is the same reading the exact lifts of this target
 *           record for those four offsets. The draw pass 0x801F42E8 is the one the
 *           neighbouring entries of this row also run before their own tests.
 *           0x801490A8 reads 0x12 here and the exact lifts of this target call it
 *           the shared status halfword; 0x80143BB0 and 0x80146875 have no
 *           established role and stay address-anchored.
 *
 *           Shape evidence: the 0xF0-byte span is not byte-identical to any span
 *           of the corpus - a scan of every payload under out/binaries found the
 *           0xF0-byte window only at offset 0x9B0 of this payload, and no window of
 *           any payload matched the span with the immediate and jump-target fields
 *           of its I- and J-format words masked either - so no clone applied and
 *           no near twin carried this body. The statement order, the two cell
 *           re-reads and the reload after the spawn call come from this span's own
 *           bytes. The 0x18-byte frame with $ra at 0x10, the cell read that the
 *           original materialises before the frame and the record byte accesses
 *           follow the exact lifts of this target.
 *
 *           Representation: the cell is spelled as this target's map name and
 *           re-read once per access group, which is the lui/lw pair the original
 *           repeats after the spawn call and again before the trigger stores; the
 *           two counter bytes are kept in u8 locals so the compared value is the
 *           one the store reuses; the trigger is the short-circuit conjunction the
 *           original's beq/bne/bne triple carries; the motion words take s32
 *           stores as the exact sibling at 0x801F3AA8 spells the same two offsets,
 *           and the two offsets it clears take u32 stores. The three address
 *           groups - the compare and its store, the spawn count, and the trigger
 *           stores - each carry their own short-lived pointer local. One pointer
 *           local shared by all three groups measured 39/60 with the first
 *           difference at +0x0000, a pure $a0/$v1 swap on every address; giving
 *           the trigger store group and the trigger byte group their own locals
 *           lifted that to 60/60, so the split is the lever that closed the
 *           register tie and the shared local is the losing spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void spawnShrinkingIntervalMatch(
    void) {
  u8* work;
  u8* base;
  u8* last;
  u8 progress;
  u8 interval;

  work = scratchWorkPointerAt1f800044;
  progress = work[0x0A];
  interval = work[0x06];
  if (progress == interval) {
    func_801F40D0();
    work = scratchWorkPointerAt1f800044;
    interval = work[0x06];
    if (interval != 0) {
      work[0x06] = interval - 2;
    }
    scratchWorkPointerAt1f800044[0x0A] = 0;
  } else {
    work[0x0A] = progress + 1;
  }
  func_801F42E8();
  if (D_80143BB0 != 2 && D_801490A8 == 0x12 && D_80146875 == 0x0C) {
    base = scratchWorkPointerAt1f800044;
    base[0x09] = 0;
    last = scratchWorkPointerAt1f800044;
    *(s32*)(base + 0x0C) = 0xA000;
    *(s32*)(base + 0x10) = 0xB000;
    *(u32*)(base + 0x18) = 0;
    *(u32*)(base + 0x1C) = 0;
    last[1]++;
  }
}
