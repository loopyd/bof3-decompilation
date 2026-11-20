#include "bof3/bof3.h"

/* Overlay handler seeded from the exact sibling
 * advanceHalfword0aByCWhenWorkByte79Not8World00Area01313_801F2CAC, which held
 * the same two-cell body: the two 0x40-byte payload spans differ in exactly the
 * gate literal word and the increment literal word, and this span's own bytes
 * supply both. The +0x79 byte of the work record is a mode selector for this
 * overlay family; mode 2 is the case that must not advance the halfword. */

/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F2C68
 * @behavior Overlay handler: reads the +0x79 byte of the work record published
 *           through the global pointer cell 0x80146250 and, when that byte is
 *           not 2, advances the halfword at offset 0x0A of the record published
 *           through the global pointer cell 0x80148210 by 9. Mode 2 returns
 *           before the halfword is touched. The halfword is read and written
 *           back through the one register that holds the 0x80148210 record.
 *           Takes no arguments, returns nothing, calls nothing and reserves no
 *           stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword0aBy9WhenWorkByte79Not2World00Area01813_801F2C68(void) {
  if (D_80146250[0x79] != 2) {
    D_80148210[5] += 9;
  }
}
