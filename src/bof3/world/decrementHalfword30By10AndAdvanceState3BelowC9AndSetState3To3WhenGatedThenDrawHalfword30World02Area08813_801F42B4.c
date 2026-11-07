#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
extern u8 D_80143BB0;
extern u16 D_80146258;

/* The local drawer at 0x801F4C28 narrows both arguments with `sll`/`sra 16`
 * in its own body and neither call site consumes a result. */
void func_801F4C28(s16 arg0, s16 arg1);

/* @source 0x801F42B4
 * @behavior Handler 1 of the four-entry state-byte-3 handler run based at
 *           0x801F61CC for the work record published at the scratchpad pointer
 *           cell 0x1F800044. Subtracts 0xA from the unsigned halfword at record
 *           offset 0x30 and stores the truncated halfword; while the stored
 *           value read back as a signed halfword stays below 0xC9 it increments
 *           work byte 0x03, through the same cell read. It then loads state byte
 *           0x03 of that record again only when the gate holds: the shared byte
 *           0x801454F2 is set and record byte 0x0B is nonzero, or byte
 *           0x80143BB0 is 2, or bit 0x100 of halfword 0x80146258 is set. On the
 *           gated path a second read of the pointer cell writes 3 to byte 0x03.
 *           Finally it reads the cell again and calls the local drawer at
 *           0x801F4C28 with 0x5C and the signed halfword at offset 0x30. Every
 *           group carries its own read of the pointer cell through the named
 *           slot symbol. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By10AndAdvanceState3BelowC9AndSetState3To3WhenGatedThenDrawHalfword30World02Area08813_801F42B4(
    void) {
  u8* record;
  u8* forced;
  u8* draw;
  s16* half;

  record = D_1F800044;
  half = (s16*)(record + 0x30);
  *half = (s16)(*half - 0xA);
  if (*half < 0xC9) {
    record[3]++;
  }
  if (D_801454F2 != 0) {
    if (D_1F800044[0x0B] != 0) {
      goto force_state;
    }
  }
  if (D_80143BB0 == 2) {
    goto force_state;
  }
  if ((D_80146258 & 0x100u) != 0) {
    goto force_state;
  }
  goto draw_record;
force_state:
  forced = D_1F800044;
  forced[3] = 3;
draw_record:
  draw = D_1F800044;
  func_801F4C28(0x5C, FIELD_REF(s16, draw, 0x30));
}
