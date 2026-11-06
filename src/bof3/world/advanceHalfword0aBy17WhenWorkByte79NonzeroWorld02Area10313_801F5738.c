#include "bof3/bof3.h"

/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F5738
 * @behavior Overlay handler: reads the +0x79 byte of the work record published
 *           through the global pointer cell 0x80146250 and, when that byte is
 *           nonzero, advances the halfword at offset 0x0A of the record
 *           published through the global pointer cell 0x80148210 by 0x17.
 *           Takes no arguments, returns nothing and uses no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword0aBy17WhenWorkByte79NonzeroWorld02Area10313_801F5738(void) {
  if (D_80146250[0x79] != 0) {
    D_80148210[5] += 0x17;
  }
}
