#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146884; /* @source 0x80146884 @kind unknown */

/* @source 0x801F47F0
 * @behavior Advances the scratch work record published through the pointer cell
 * at 0x1F800044: adds its 0x0C step word to the 0x34 accumulator, adds its 0x10
 * step word to the 0x38 accumulator, and then steps each of the signed level
 * bytes 0x5D, 0x5E and 0x5F by 2 while it stays below -0x40. When the word at
 * 0x5C carries the 0xC0C0C000 colour marker it clears flag bit 0x20 of record
 * byte 0, then zeroes the 0x5C marker byte, the 0x5D-0x5F level triple and the
 * handler byte at 0x04, each after a fresh load of the cell. Otherwise it
 * consumes two units of the countdown halfword at 0x7E of the work record
 * published through the global pointer cell 0x80146884. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The three level blocks read the cell once for the test and the add and then
 * reload it before the next level, exactly as the original does, so the 0x5D
 * block stays on the record local that the accumulator updates already loaded
 * while 0x5E and 0x5F go through the named cell. The original reserves 0x18
 * bytes of frame with no saved register and no sp-relative access, and its
 * adjustment lands in the 0x5D compare's delay slot; this body's own locals
 * reproduce that reservation and placement without a declared slot.
 */
void advanceScratchWorkLevelTripleAndAccumulatorsAtColorMarkerWorld04Area17213_801F47F0(void) {
  u8* record;
  u8* cursor;
  u8* work;

  record = D_1F800044;
  *(s32*)(record + 0x34) += *(s32*)(record + 0x0c);
  *(s32*)(record + 0x38) += *(s32*)(record + 0x10);
  if (*(s8*)(record + 0x5d) < -0x40) {
    *(s8*)(record + 0x5d) += 2;
  }
  if (((s8*)D_1F800044)[0x5e] < -0x40) {
    ((s8*)D_1F800044)[0x5e] += 2;
  }
  if (((s8*)D_1F800044)[0x5f] < -0x40) {
    ((s8*)D_1F800044)[0x5f] += 2;
  }

  record = D_1F800044;
  if ((*(u32*)(record + 0x5c) & 0xffffff00) == 0xc0c0c000) {
    record[0] &= 0xdf;
    D_1F800044[0x5c] = 0;
    cursor = D_1F800044;
    cursor[0x5f] = 0;
    cursor[0x5e] = 0;
    cursor[0x5d] = 0;
    D_1F800044[4] = 0;
  } else {
    work = D_80146884;
    *(u16*)(work + 0x7e) -= 2;
  }
}
