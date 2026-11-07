#include "bof3/bof3.h"

/* Overlay handler shaped by the exact sibling
 * advanceHalfword0aBy17WhenWorkByte79NonzeroWorld02Area10313_801F5738, which
 * holds the same two-cell body and differs only in its gate literal and its
 * stored increment. The +0x79 byte of the work record is a mode selector for
 * this overlay family; mode 8 is the case that must not advance the halfword. */

/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F2CAC
 * @behavior Overlay handler: reads the +0x79 byte of the work record published
 *           through the global pointer cell 0x80146250 and, when that byte is
 *           not 8, advances the halfword at offset 0x0A of the record published
 *           through the global pointer cell 0x80148210 by 0x0C. Mode 8 returns
 *           before the halfword is touched. The halfword is read and written
 *           back through the one register holding the 0x80148210 record. Takes
 *           no arguments, returns nothing and uses no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword0aByCWhenWorkByte79Not8World00Area01313_801F2CAC(void) {
  if (D_80146250[0x79] != 8) {
    D_80148210[5] += 0x0C;
  }
}
