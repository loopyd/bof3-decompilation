#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801C38F4
 * @behavior counts how many bytes of one 164-byte record of the table at
 * D_80144968 equal the low byte of arg2: the record is selected by the byte
 * D_80181B10[arg0 & 0xFF] and the compared group of record bytes is selected by
 * the low byte of arg1, where 1 compares byte 0x0E, 2 compares bytes 0x0F, 0x10
 * and 0x11, and 3 compares bytes 0x12 and 0x13; the first byte of a group counts
 * as one and every further match adds one, while an unlisted group returns 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801C38F4(s32 arg0, s32 arg1, s32 arg2)
{
  u8* record = &D_80144968[D_80181B10[arg0 & 0xFF] * 164];
  s32 count = 0;

  switch (arg1 & 0xFF) {
  case 0:
    break;
  case 1:
    if (record[0x0E] == (arg2 & 0xFF)) {
      count = 1;
    }
    break;
  case 2:
    if (record[0x0F] == (arg2 & 0xFF)) {
      count = 1;
    }
    if (record[0x10] == (arg2 & 0xFF)) {
      count++;
    }
    if (record[0x11] == (arg2 & 0xFF)) {
      count++;
    }
    break;
  case 3:
    if (record[0x12] == (arg2 & 0xFF)) {
      count = 1;
    }
    if (record[0x13] == (arg2 & 0xFF)) {
      count++;
    }
    break;
  }
  return count;
}
