#include "bof3/bof3.h"

/* Scratchpad pointer cell holding the work record this step runs on. Spelled as
 * the named fixed address rather than through the constant-address accessor,
 * because the original re-emits `lui $vN,0x1F80` plus the folded
 * `lw $vN,68($vN)` pair before every access group; the accessor form shares one
 * materialised base across them. The `D_<ADDRESS>` spelling binds with no map
 * row, the same declaration the neighbouring lifts at 0x801F3588 and 0x801F35E4
 * use.
 * @source 0x1F800044 @kind unknown
 */
extern u8* D_1F800044;

/* Pointer cell of the shared record whose halfword +0x0A this step counts down
 * by two while the work record tint channels have not reached their floor. The
 * cell is bound by this target's generated `undefined_syms_auto.txt`, and the
 * exact lift at 0x801F35E4 reads the same pointer for its byte +0x07, so the
 * address keeps that spelling with no map row.
 * @source 0x80148210 @kind unknown
 */
extern u8* D_80148210;

/* @source 0x801F3740
 * @behavior Handler index 1 of the state-byte-4 run of this overlay: the
 *           payload handler table at 0x801F4DC4 holds this address at word 1,
 *           so the body takes no arguments and returns nothing.
 *           It loads the work record published through the scratchpad pointer
 *           cell 0x1F800044 and adds the record word +0x0C to its position word
 *           +0x34 and the record word +0x10 to its position word +0x38, both as
 *           unsigned words and in that order. It then steps the three signed
 *           tint channel bytes +0x5D, +0x5E and +0x5F down by two, skipping a
 *           channel that already reads -0x80 so the floor is never crossed.
 *           The +0x5D group runs on the first published record; the +0x5E group
 *           re-reads the cell before its test and again after its store, because
 *           a store through the loaded pointer may alias the cell itself, and
 *           the +0x5F group runs on the pointer that last read left.
 *           Finally it re-reads the cell and tests the flag word at +0x5C with
 *           0xFFFFFF00 against 0x80808000: when all three channel bytes read
 *           0x80 the tint has reached its floor, so the body sets bit 0x40 of
 *           the record flag byte +0x00, re-reads the cell and stores zero into
 *           the record state byte +0x04, which returns the record to handler
 *           index 0 of the same run. Otherwise it reads the shared record
 *           published at the pointer cell 0x80148210 and decrements that
 *           record's halfword +0x0A by two. Nothing is returned.
 *
 * Naming evidence: the state-byte-4 dispatcher of this overlay,
 *   dispatchStateByte4Table4dc4World03Area14113_801F3688, holds the handler
 *   table at 0x801F4DC4 whose four words are 0x801F36CC, this address,
 *   0x801F387C and 0x801F390C, and it indexes that table with the record byte
 *   +0x04, so the zero this body stores into +0x04 re-arms handler index 0 and
 *   the body is the second step of that run. The channel triplet at 0x5D-0x5F
 *   and the -0x80 floor are the tint channels the exact main-executable lifts
 *   func_801C29A0 and func_801B72B0 name as the record tint, and the flag word
 *   mask 0x80808000 is the all-three-channels-at-the-floor test func_801C29A0
 *   returns. Bit 0x40 of the record flag byte has no proven role beyond this
 *   body raising it at the floor, so the name carries the literal.
 *
 * Shape evidence: the channel groups copy the statement order, the signed byte
 *   spelling and the operand forms of the exact lift at 0x801F3588, which walks
 *   the same three channels of the same record in the neighbour arm of this
 *   overlay, and the position-word form and the cell-reload style come from the
 *   exact lift at 0x801F35E4. Three things are this span's own: the +0x5E group
 *   re-reads the cell inside its own branch after the store, the +0x5F group
 *   runs on that read, and the three pointer reads sit in three separate locals,
 *   which is what puts them in $a1, $a0 and $v0 - one local for all of them keeps
 *   $a1 for every read and left four register rows of the second half differing
 *   at the same instruction count.
 *
 * Frame: the original reserves a 0x18-byte frame and touches none of it; the
 *   same declarations reproduce that size with no padding local, so the slot is
 *   the compiler's own, not a recovered declaration.
 *
 * Clone declined: a payload scan for the 0x28-byte signature of this body
 *   across the byte images under out/binaries found it only in this payload, so
 *   no byte-identical twin exists to clone.
 *
 * Surplus bytes: none - the row runs from 0x801F3740 to the next boundary at
 *   0x801F3838, and the body's own `jr $ra`/delay pair ends on that boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkPositionAndFadeTintTripletThenSetFlag40AndRearmHandlerZeroAtFloorElseDecrementHalfwordAWorld03Area14113_801F3740(
    void) {
  u8* work;
  u8* tint;
  u8* cell;

  work = D_1F800044;
  FIELD_REF(u32, work, 0x34) += FIELD_REF(u32, work, 0x0C);
  FIELD_REF(u32, work, 0x38) += FIELD_REF(u32, work, 0x10);
  if (FIELD_REF(s8, work, 0x5D) != -0x80) {
    FIELD_REF(s8, work, 0x5D) -= 2;
  }
  tint = D_1F800044;
  if (FIELD_REF(s8, tint, 0x5E) != -0x80) {
    FIELD_REF(s8, tint, 0x5E) -= 2;
    tint = D_1F800044;
  }
  if (FIELD_REF(s8, tint, 0x5F) != -0x80) {
    FIELD_REF(s8, tint, 0x5F) -= 2;
  }
  work = D_1F800044;
  if ((FIELD_REF(u32, work, 0x5C) & 0xFFFFFF00u) == 0x80808000u) {
    FIELD_REF(u8, work, 0x00) |= 0x40;
    cell = D_1F800044;
    FIELD_REF(u8, cell, 0x04) = 0;
  } else {
    FIELD_REF(u16, D_80148210, 0x0A) -= 2;
  }
}
