#include "bof3/bof3.h"

/* @source 0x801F35F0
 * @behavior Reads the work byte at +8 of the record published at scratchpad
 *           pointer cell 0x1F800044 into a byte local and writes 3 back when
 *           that value already reads 7, otherwise writes 7. The pointer cell is
 *           loaded once and that one base serves both the byte read and the
 *           byte store, and the byte is read once because the comparison uses
 *           the loaded value rather than a second read of the record. The role
 *           of the +8 byte stays anchored to its offset. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte8To3When7Else7World02Area09513_801F35F0(void) {
  u8* work;
  u8 current;

  work = SPAD_PTR_SLOT(u8, 0x44);
  current = work[8];
  work[8] = (current == 7) ? 3 : 7;
}
