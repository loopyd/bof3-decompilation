#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original reads the cell twice: once
 * for the position triple and the step it derives from the object's other
 * triple, and again for the two points it projects for the record heading, so
 * the body keeps one pointer local per read.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Heading helper of this payload, still on its assembled boundary: it projects
 * each of the two three-word points through 0x801AFF04 and returns the 12-bit
 * heading between the two projections, masked to 16 bits.
 * @source 0x801F3320 @kind function
 */
extern u16 func_801F3320(u8* from, u8* to);

/* @source 0x801F3378
 * @behavior Claims one record of the 0x20-entry, 0x2C-byte-stride work table at
 * 0x800E4800 - the record its caller obtained from the claim scan at
 * 0x801F2E88 - by writing 1 to the record's occupancy byte at offset 3, and
 * clears its byte at offset 4. It then seeds the record from the scratch work
 * object published through the pointer cell at 0x1F800044: the position words
 * at 0x0C, 0x10, 0x14 take the work object's words at 0x34, 0x38 and 0x3C
 * unchanged, while the three step words at 0x1C, 0x20 and 0x24 take the
 * difference to that position triple from the work object's other triple at
 * 0x0C, 0x10 and 0x14, arithmetic-shifted right by five, so each step is one
 * thirty-second of that difference. Halfword 0x08 takes 0x40 and halfword 0x0A
 * takes 0x20, which the per-frame stepper at 0x801F2EC4 passes to the quad
 * emitter 0x801F2FEC as the record's size pair, and byte 5 takes 0x20, the
 * countdown that stepper decrements once per frame before it frees the record
 * when the byte reaches zero. Last, the scratch work pointer is read a second
 * time and the overlay helper 0x801F3320, which projects the two three-word
 * points and returns the heading between their projections, supplies halfword
 * 0x06 - the heading the stepper recomputes for every active record and hands
 * to the emitter with the record's position. The body takes the record pointer
 * in $a0 and returns nothing, so a caller that discards the answer keeps only
 * the record it passed. Its 0x18-byte frame saves $ra and the record in $s0
 * across the one call; the second pointer read lands in the argument registers
 * directly, so it costs no saved register. The two position copies and the
 * three steps are read once each from the first pointer, and no other memory is
 * touched.
 *
 * Name evidence: this target's claim scan
 * findFreeWorkRecordByte3At800E4800World04Area17413_801F2E88 returns the first
 * record of that table whose byte 3 reads zero, and both of its callers in this
 * payload - jal 0x801F2E88 at 0x801F2CCC inside func_801F2CA8 and at 0x801F2D8C
 * inside func_801F2D68 - discard a zero answer and otherwise pass the returned
 * pointer here in $a0. The stepper func_801F2EC4 in the same payload walks the
 * same table and establishes the field roles this body writes: it skips a
 * record whose byte at offset 3 is zero, adds the words at 0x1C, 0x20 and 0x24
 * into the position words at 0x0C, 0x10 and 0x14, stores the answer of
 * 0x801F3320 into halfword 0x06, decrements the countdown at byte 5, clears
 * byte 3 when that countdown reaches zero, and passes halfwords 0x08 and 0x0A
 * with halfword 0x06 as the emitter's size pair and heading. The scratch work
 * object word triple 0x34/0x38/0x3C is the position triple of this overlay and
 * of the same-shaped seeds in emi/scenario, and the objects that carry it are
 * distinct: bytes 0x3B0.. of emi/world00/area028/13 and bytes 0x21BC.. of
 * emi/world02/area098/13 open with the same prologue and diverge at the first
 * statement, where those copies randomize the position instead of stepping it,
 * and no other payload carries this body's head, so this target owns its own
 * source, address and boundary. The work object's remaining triple at
 * 0x0C/0x10/0x14 is the second point 0x801F3320 projects, so the step words
 * point from the record's seeded position toward it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimWorkRecordAndSeedPositionStepSizeAndHeadingFromWorkObjectWorld04Area17413_801F3378(
    u8* record) {
  u8* work;
  u8* next;
  s32 step;

  record[3] = 1;
  record[4] = 0;
  work = D_1F800044;
  *(u32*)(record + 0xC) = *(u32*)(work + 0x34);
  *(u32*)(record + 0x10) = *(u32*)(work + 0x38);
  *(u32*)(record + 0x14) = *(u32*)(work + 0x3C);
  *(s32*)(record + 0x1C) = ((s32)(*(u32*)(work + 0xC) - *(u32*)(work + 0x34))) >> 5;
  *(s32*)(record + 0x20) = ((s32)(*(u32*)(work + 0x10) - *(u32*)(work + 0x38))) >> 5;
  step = ((s32)(*(u32*)(work + 0x14) - *(u32*)(work + 0x3C))) >> 5;
  *(u16*)(record + 0x8) = 0x40;
  *(u16*)(record + 0xA) = 0x20;
  record[5] = 0x20;
  next = D_1F800044;
  *(s32*)(record + 0x24) = step;
  *(u16*)(record + 0x6) = func_801F3320(next + 0x34, next + 0xC);
}
