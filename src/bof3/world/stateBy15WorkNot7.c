#include "bof3/bof3.h"

/* Seeded from advanceHalfword0aByCWhenWorkByte79Not8World00Area01313_801F2CAC,
 * an exact lift in the neighbouring AREA013 overlay 0x801F2CAC span. One payload
 * comparison of the two 0x40-byte bodies differed in exactly two bytes: the gate
 * immediate word and the stored increment immediate word. Every other word,
 * including the delay-slot nops, stayed equal, so the statement order and operand
 * forms are copied and this span's own literals are written here.
 *
 * The +0x79 byte of the work record is a mode selector for this overlay family;
 * mode 7 is the case that must not advance the halfword. */

/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F2CDC
 * @behavior Overlay handler: reads the +0x79 byte of the work record published
 *           through the global pointer cell 0x80146250 and, when that byte is
 *           not 7, advances the halfword at offset 0x0A of the record published
 *           through the global pointer cell 0x80148210 by 0x15. Mode 7 returns
 *           before the halfword is touched. The halfword is read and written
 *           back through the one register holding the 0x80148210 record. Takes
 *           no arguments, returns nothing and uses no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateBy15WorkNot7(void) {
  if (D_80146250[0x79] != 7) {
    D_80148210[5] += 0x15;
  }
}
