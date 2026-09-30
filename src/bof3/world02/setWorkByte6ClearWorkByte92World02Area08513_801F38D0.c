#include "bof3/bof3.h"

extern u8* D_80146884;

/* @source 0x801F38D0
 * @behavior Overlay setter: writes 7 to byte +0x6 of the work record reached
 *           through scratchpad pointer-slot 0x1F800044, then clears byte
 *           +0x92 of the record reached through the current-record cursor
 *           D_80146884; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte6ClearWorkByte92World02Area08513_801F38D0(void) {
  u8* work;
  u8* record;

  work = SPAD_PTR_TABLE(u8)[0x11];
  work[6] = 7;
  record = D_80146884;
  record[0x92] = 0;
}
