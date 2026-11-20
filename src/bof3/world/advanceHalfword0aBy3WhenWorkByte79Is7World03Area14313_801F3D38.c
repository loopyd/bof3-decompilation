#include "bof3/bof3.h"

/* Overlay handler seeded from the shape of
 * advanceHalfword0aBy9WhenWorkByte79Not2World00Area01813_801F2C68, whose
 * 0x40-byte payload span shares every word with this one except three: the gate
 * literal, the test opcode and the increment literal. Those three come from this
 * span's own bytes, which test the +0x79 work byte for equality with 7 instead
 * of inequality. Both halves of the body publish their record through a global
 * pointer cell outside the payload, so the two cells keep the sibling's names
 * and binding kind. */

/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F3D38
 * @behavior Overlay handler: reads the +0x79 byte of the work record published
 *           through the global pointer cell 0x80146250 and, when that byte
 *           equals 7, advances the halfword at offset 0x0A of the record
 *           published through the global pointer cell 0x80148210 by 3. Every
 *           other value of the byte returns before the halfword is touched. The
 *           halfword is read and written back through the one register that
 *           holds the 0x80148210 record. Takes no arguments, returns nothing,
 *           calls nothing and reserves no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword0aBy3WhenWorkByte79Is7World03Area14313_801F3D38(void) {
  if (D_80146250[0x79] == 7) {
    D_80148210[5] += 3;
  }
}
