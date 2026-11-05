#include "bof3/bof3.h"

extern s16 D_8014932C;
extern u16* D_80148210;
extern u8 D_80149333;

/* @source 0x801F2CBC
 * @behavior If the signed halfword at 0x8014932C is below 1500, add 64
 * to it and subtract 2 from the halfword at offset 0x0A of the record
 * published at 0x80148210. Otherwise set the shared halfword to 1500
 * and clear byte 4 of the scratchpad work record. Finally store 2 at
 * 0x80149333. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword932cTo1500World03Area12114_801F2CBC(void) {
  s16 value;
  u16* record;

  value = D_8014932C;
  if (value < 1500) {
    record = D_80148210;
    D_8014932C = value + 64;
    record[5] -= 2;
  } else {
    D_8014932C = 1500;
    SPAD_PTR_SLOT(u8, 0x44)[4] = 0;
  }
  D_80149333 = 2;
}
