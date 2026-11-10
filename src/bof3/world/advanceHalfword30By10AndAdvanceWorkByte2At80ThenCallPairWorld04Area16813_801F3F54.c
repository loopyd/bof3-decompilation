#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area16813WorkCursor_1F800044;

/* @source 0x801F4158 @kind function */
extern void func_801F4158(s32 index, s8 mode);

/* @source 0x801F3F54
 * @behavior Reads the scratchpad work pointer at 0x1F800044, adds 0x10 to
 *           halfword +0x30, and advances dispatch byte +2 when the updated
 *           signed halfword is at least 0x80. Then calls local helper
 *           0x801F4158 with (0, 0) and (1, 3). Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By10AndAdvanceWorkByte2At80ThenCallPairWorld04Area16813_801F3F54(void) {
  u8* work;
  s32 offset;

  work = area16813WorkCursor_1F800044;
  offset = FIELD_REF(u16, work, 0x30) + 0x10;
  FIELD_REF(u16, work, 0x30) = offset;
  if ((s16)offset >= 0x80) {
    work[2]++;
  }
  func_801F4158(0, 0);
  func_801F4158(1, 3);
}
