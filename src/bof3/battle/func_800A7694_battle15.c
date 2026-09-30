#include "bof3/battle/battle15_internal.h"

/* @source 0x800A7694
 * @behavior Writes the nine class-slot bytes 0x801EBEFA..0x801EBF02 and their
 *   copies at record + 0x2B..0x33 and record + 0x4B..0x53, where record is the
 *   active battler record 0x80145F04 + D_80146374 * 0x140. Each of the five
 *   signed index arguments is offset by two and clamped to 0..4, then indexes a
 *   local five-entry byte table: slots 0 through 2 read 0x800B4E94 with the
 *   first three arguments, slot 5 reads 0x800B4E9E with the fifth argument and
 *   slots 7 and 8 read 0x800B4E99 with the fourth argument (re-read after the
 *   first slot 7 stores); slots 3, 4 and 6 store the constant 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A7694(s8 a0, s8 a1, s8 a2, s8 a3, s8 a4) {
  u8* record;

  record = &D_80145F04[(u32)D_80146374 * 0x140u];

  {
    s8 row;
    u8 value;
    row = a0;
    row += 2;
    if (row < 0) {
      row = 0;
    }
    if (row >= 5) {
      row = 4;
    }
    value = D_800B4E94[row];
    record[0x4B] = value;
    record[0x2B] = value;
    D_801EBEFA[0] = value;
  }
  {
    s8 row;
    u8 value;
    row = a1;
    row += 2;
    if (row < 0) {
      row = 0;
    }
    if (row >= 5) {
      row = 4;
    }
    value = D_800B4E94[row];
    record[0x4C] = value;
    record[0x2C] = value;
    D_801EBEFA[1] = value;
  }
  {
    s8 row;
    u8 value;
    row = a2;
    row += 2;
    if (row < 0) {
      row = 0;
    }
    if (row >= 5) {
      row = 4;
    }
    value = D_800B4E94[row];
    record[0x4D] = value;
    record[0x2D] = value;
    D_801EBEFA[2] = value;
  }
  record[0x4E] = 2;
  record[0x2E] = 2;
  D_801EBEFA[3] = 2;
  record[0x4F] = 2;
  record[0x2F] = 2;
  D_801EBEFA[4] = 2;
  {
    s8 row;
    u8 value;
    row = a4;
    row += 2;
    if (row < 0) {
      row = 0;
    }
    if (row >= 5) {
      row = 4;
    }
    value = D_800B4E9E[row];
    record[0x50] = value;
    record[0x30] = value;
    D_801EBEFA[5] = value;
  }
  record[0x51] = 2;
  record[0x31] = 2;
  D_801EBEFA[6] = 2;
  {
    s8 row;
    u8 value;
    u8 second;
    row = a3;
    row += 2;
    if (row < 0) {
      row = 0;
    }
    if (row >= 5) {
      row = 4;
    }
    value = D_800B4E99[row];
    record[0x52] = value;
    record[0x32] = value;
    D_801EBEFA[7] = value;
    second = D_800B4E99[row];
    record[0x53] = second;
    record[0x33] = second;
    D_801EBEFA[8] = second;
  }
}
