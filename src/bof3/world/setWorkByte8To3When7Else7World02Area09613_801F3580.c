#include "bof3/bof3.h"

/* @source 0x801F3580
 * @behavior Reads the work record published at scratchpad pointer cell
 * 0x1F800044, loads its byte 0x08 once and stores a selection back into that
 * same byte: 3 when the loaded byte was 7, otherwise 7. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte8To3When7Else7World02Area09613_801F3580(void) {
  s32 value;
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  value = work[8];
  work[8] = (value == 7) ? 3 : 7;
}
