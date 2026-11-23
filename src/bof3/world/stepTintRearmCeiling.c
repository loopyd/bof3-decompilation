#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Spelled as the named fixed address
 * rather than through the constant-address accessor, because the original
 * re-emits `lui $vN,0x1F80` plus the folded `lw $vN,68($vN)` pair before every
 * access group; the accessor form shares one materialised base across them.
 * The `D_<ADDRESS>` spelling binds with no map row, the same declaration the
 * neighbouring lifts at 0x801F3588, 0x801F35E4 and 0x801F3740 use.
 * @source 0x1F800044 @kind unknown
 */
extern u8* D_1F800044;

/* Pointer cell of the shared record whose halfword +0x7E this step counts down
 * by two while the work record tint channels have not reached their ceiling.
 * The cell is bound by this target's generated `undefined_syms_auto.txt`, the
 * same binding the exact lift at 0x801F35E4 reads for its record byte +0x07.
 * @source 0x80146884 @kind unknown
 */
extern u8* D_80146884;

/* @source 0x801F390C
 * @behavior Handler index 3 of the state-byte-4 run of this overlay: the
 *           payload handler table at 0x801F4DC4 holds this address at word 3,
 *           and the shorter run based at 0x801F4DCC holds it at word 1, so the
 *           body takes no arguments and returns nothing.
 *           It loads the work record published through the scratchpad pointer
 *           cell 0x1F800044 and adds the record word +0x0C to its position word
 *           +0x34 and the record word +0x10 to its position word +0x38, both as
 *           unsigned words and in that order. It then raises the three signed
 *           tint channel bytes +0x5D, +0x5E and +0x5F by four, skipping a
 *           channel that already reads -0x40 so that ceiling is never crossed.
 *           The +0x5D group runs on the first published record; the +0x5E group
 *           re-reads the cell after its store, because a store through the
 *           loaded pointer may alias the cell itself, and the +0x5F group runs
 *           on the pointer that read left.
 *           Finally it re-reads the cell and tests the flag word at +0x5C with
 *           0xFFFFFF00 against 0xC0C0C000: when all three channel bytes read
 *           0xC0 the tint has reached its ceiling, so the body clears bit 0x20
 *           of the record flag byte +0x00, re-reads the cell and stores zero
 *           into the record mode byte +0x5C, re-reads it once more and zeroes
 *           the channel bytes +0x5F, +0x5E and +0x5D in that order, then
 *           re-reads it a last time and stores zero into the record state byte
 *           +0x04, which returns the record to handler index 0 of the same run.
 *           Otherwise it reads the shared record published at the pointer cell
 *           0x80146884 and decrements that record's halfword +0x7E by two.
 *           Nothing is returned, and its 0x18-byte frame reserves bytes no
 *           access of the body touches; the record pointer load precedes the
 *           frame creation.
 *
 * Naming evidence: the state-byte-4 dispatcher of this overlay,
 *   callStateByte4Table4, indexes the handler
 *   table at 0x801F4DC4 with the record byte +0x04, and that table's four words
 *   are 0x801F36CC, 0x801F3740, 0x801F387C and this address, so the zero this
 *   body stores into +0x04 re-arms handler index 0 and the body is the fourth
 *   and last step of that run. The 0x5D to 0x5F channel triplet, the -0x40
 *   ceiling and the masked 0xC0C0C0 flag word are the same tint idiom the exact
 *   lift at 0x801F3740 walks down to its -0x80 floor and the exact lifts at
 *   0x801F3FFC in target emi/world04/area174/13 and 0x801F3920 in target
 *   emi/world00/area032/13 raise to that same ceiling. Bit 0x20 of the record
 *   flag byte has no proven role beyond this body resetting it at the ceiling,
 *   so the name carries the literal.
 *
 * Shape evidence: the position-word form, the signed byte channel spelling and
 *   the operand forms come from the exact lift at 0x801F3740, the neighbour
 *   handler of this overlay that walks the same record in the opposite
 *   direction. The ceiling arm's statement set - the bit-0x20 reset, the mode
 *   byte, the three channel zeroes in +0x5F, +0x5E, +0x5D order and the shared
 *   record halfword countdown - comes from the exact lift at 0x801F3FFC in
 *   target emi/world04/area174/13, whose own payload carries the same idiom
 *   with a halfword test. This span's own statements are the two position word
 *   updates, the four-unit channel step, the state-byte +0x04 store and the
 *   cell-reusing bit-0x20 reset, which the neighbour in target
 *   emi/world00/area032/13 spells as a fresh cell read instead; every one of
 *   them is written from this span's own bytes.
 *
 * Locals and cell reads: the +0x5E and +0x5F groups read the cell inline and
 *   the ceiling arm's three reloads do too, while one local holds the +0x5F,
 *   +0x5E, +0x5D zero run. Named locals measured worse: one local for the two
 *   groups and the arm kept the +0x5E pointer in $v1 where the original holds
 *   it in $a0 and matched 52 of 75 rows, splitting the group local out of it
 *   still left the first difference on that row at 60 of 75, and inlining the
 *   two groups while the arm kept its reloads in locals left the first
 *   difference on the arm's third reload at 72 of 75. Only the inline arm
 *   reloads put all three in $v0 and matched.
 *
 * Frame: the original reserves a 0x18-byte frame, creates it in the delay slot
 *   of the first channel test and touches none of it; the same declarations
 *   reproduce that placement with no padding local.
 *
 * Clone declined: a payload scan of every image under out/binaries for this
 *   body's 0x12C-byte span found no byte-identical twin. The nearest neighbour
 *   is the body of 0x801F3920 in target emi/world00/area032/13, equal over the
 *   27 words from this span's +0x5E tint group through the flag-test mask and
 *   different on either side of that run, so it is shared-idiom shape evidence,
 *   not a clone source.
 *
 * Surplus bytes: none - the row runs from 0x801F390C to the next boundary at
 *   0x801F3A38, and the body's own `jr $ra`/delay pair ends on that boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepTintRearmCeiling(
    void) {
  u8* work;
  u8* tint;
  u8* cell;

  work = D_1F800044;
  FIELD_REF(u32, work, 0x34) += FIELD_REF(u32, work, 0x0C);
  FIELD_REF(u32, work, 0x38) += FIELD_REF(u32, work, 0x10);
  if (FIELD_REF(s8, work, 0x5D) < -0x40) {
    FIELD_REF(s8, work, 0x5D) += 4;
  }
  if (((s8*)D_1F800044)[0x5E] < -0x40) {
    ((s8*)D_1F800044)[0x5E] += 4;
  }
  if (((s8*)D_1F800044)[0x5F] < -0x40) {
    ((s8*)D_1F800044)[0x5F] += 4;
  }
  work = D_1F800044;
  if ((FIELD_REF(u32, work, 0x5C) & 0xFFFFFF00u) == 0xC0C0C000u) {
    FIELD_REF(u8, work, 0x00) &= 0xDF;
    D_1F800044[0x5C] = 0;
    tint = D_1F800044;
    FIELD_REF(u8, tint, 0x5F) = 0;
    FIELD_REF(u8, tint, 0x5E) = 0;
    FIELD_REF(u8, tint, 0x5D) = 0;
    D_1F800044[0x04] = 0;
  } else {
    cell = D_80146884;
    FIELD_REF(u16, cell, 0x7E) -= 2;
  }
}
