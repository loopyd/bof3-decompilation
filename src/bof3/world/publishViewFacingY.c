#include "bof3/bof3.h"

/* Work record published through the scratchpad pointer cell 0x1F800044. The
 * original re-materialises the cell with its own `lui 0x1F80` plus folded
 * `lw 68` pair before each of its three access groups - the first reads the
 * record's word +0x34, the second clears its word +0x14 and the third stores
 * 0x3E0000 into its word +0x38 - so the cell keeps a named spelling that every
 * group re-reads instead of one hoisted base ($v0 for the first two reads, $v1
 * for the third). The exact sibling lifts 0x801F35A0, 0x801F3AF8 and 0x801F3BA4
 * of this target declare the same cell.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Record published behind the pointer cell at 0x80148210. The original re-reads
 * the cell after the shared view call, so each access goes through its own load:
 * this body stores the scaled work coordinate into byte +0x07 and sets bit 0x40
 * of byte 0. The exact lift 0x801F37C0 of this target and the exact World01
 * area048 lifts 0x801F48DC and 0x801F49C8 read the same cell.
 * @source 0x80148210 @kind unknown
 */
extern u8* D_80148210;

/* Shared facing halfword of the main executable. It is cleared here together
 * with the work record's word +0x14, the pair the exact World01 area048 lifts
 * 0x801F48DC and 0x801F49C8 and the World03 reset step 0x801F564C also clear.
 * The original stores it with `sh $zero`, so the cell is declared at halfword
 * width.
 * @source 0x80149330 @kind data
 */
extern u16 D_80149330;

/* Shared packet-submission Y word of the main executable. The exact lift
 * 0x8019BFD4 copies the base work record's word +0x38 (0x80145EC8) into it, and
 * the World03 area119 lifts compare that same work word against 0x3E0000, which
 * is the value this step stores into both cells.
 * @source 0x8014930C @kind data
 */
extern s32 D_8014930C;

/* Shared main-executable view routine; its byte argument selects the view mode
 * it scales the view step with. The exact World03 area144 lift 0x801F3C78 and
 * the exact World01 area048 lifts 0x801F48DC and 0x801F49C8 call it for that
 * purpose, and this body passes its own mode literal 7.
 * @source 0x801C5DFC
 */
extern void func_801C5DFC(u8 mode);

/* @source 0x801F373C
 * @behavior Overlay step handler reached from data rather than from a call, so
 *           it takes no arguments and returns nothing. It loads the work record
 *           published at the scratchpad pointer cell 0x1F800044, forms the
 *           record's signed word +0x34 minus 0x160000 and shifts that difference
 *           right by 15, and stores the low byte of the result into byte +0x07
 *           of the record published at 0x80148210. It then calls the shared view
 *           routine 0x801C5DFC with the mode literal 7, clears the shared facing
 *           halfword 0x80149330, re-reads the scratchpad cell and clears the
 *           work record's word +0x14, sets bit 0x40 of byte 0 of the record
 *           published at 0x80148210, and finally stores 0x3E0000 into the shared
 *           packet Y word 0x8014930C and, after a third read of the scratchpad
 *           cell, into the work record's word +0x38. Its 0x18-byte frame saves
 *           $ra at +0x10 and holds a single plain return path.
 *
 * Naming evidence: byte +0x07 of the record published at 0x80148210 is the byte
 *   the exact World01 area048 lifts write a scaled value into (a shifted mode
 *   there, the remapped +0x34 coordinate word at World03 area141 0x801F35E4)
 *   and func_801C5DFC is the shared view routine those lifts and the World03
 *   area144 lift 0x801F3C78 name. The shared halfword 0x80149330 with the work
 *   record's word +0x14 are the facing pair the same lifts clear, and
 *   0x80149308/0x8014930C are the shared packet-submission coordinate pair the
 *   bmagic lifts name, with 0x8014930C fed from the base work record's word
 *   +0x38 (the y of the position triplet) at 0x8019BFD4 - the same word this
 *   step sets to the same 0x3E0000 literal. Byte +0x08 of the work record is
 *   the level index the area048 lifts store; this body stores no level, so the
 *   role of its own 0x160000 bias stays unproven and the literal is kept as the
 *   original constant.
 *
 * Caller evidence: no `jal` in this payload targets 0x801F373C, no branch
 *   reaches it, and no word of the payload holds its address - not even in the
 *   textbin data tail from 0x801F3D2C - so the parameter roles stay structural
 *   and the body takes none.
 *
 * Boundary: the row runs from 0x801F373C to the next boundary at 0x801F37C0 and
 *   this body's own `jr $ra`/delay pair ends it exactly there, with no surplus
 *   bytes and no leading fragment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishViewFacingY(
    void) {
  u8* work;
  u8* record;
  u8* tail;

  D_80148210[7] = (s32)(FIELD_REF(u32, D_1F800044, 0x34) - 0x160000u) >> 15;
  func_801C5DFC(7);
  work = D_1F800044;
  record = D_80148210;
  D_80149330 = 0;
  *(u32*)(work + 0x14) = 0;
  record[0] |= 0x40;
  tail = D_1F800044;
  D_8014930C = 0x3E0000;
  *(u32*)(tail + 0x38) = 0x3E0000;
}
