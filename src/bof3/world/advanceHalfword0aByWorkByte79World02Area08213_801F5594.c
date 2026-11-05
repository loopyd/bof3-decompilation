#include "bof3/bof3.h"

/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F5594
 * @behavior Overlay handler: reads the +0x79 byte of the work record published
 *           through the global pointer cell 0x80146250 and advances the
 *           halfword at offset 0x0A of the record published through the global
 *           pointer cell 0x80148210 by a value selected from that byte - 0x0A
 *           when it is 4, 0x16 for every other value, and nothing at all when
 *           it is 8 because that case returns before touching the halfword.
 *           Each arm reloads the 0x80148210 cell and re-reads the halfword, and
 *           the two arms share the single trailing halfword store. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword0aByWorkByte79World02Area08213_801F5594(void) {
  u8 state;

  state = D_80146250[0x79];
  if (state == 8) {
    return;
  }
  if (state == 4) {
    D_80148210[5] += 0x0A;
  } else {
    D_80148210[5] += 0x16;
  }
}
