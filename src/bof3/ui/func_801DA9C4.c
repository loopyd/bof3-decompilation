#include "bof3/ui/shop00_internal.h"

/* @source 0x801DA9C4
 * @behavior selects one of four main-RAM shop value tables by the mode byte
 *           and returns the table's u16 record value for the index byte;
 *           modes other than 1, 2 or 3 share the mode 0 table (the original
 *           emits their common body once, at the switch's default arm).
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit is instruction- and byte-exact.
 */
u16 func_801DA9C4(u8 mode, u8 index) {
  u16 value;

  switch (mode) {
  default:
  case 0:
    value = D_801C8974[index * 9];
    break;
  case 1:
    value = D_801C90F2[index * 12];
    break;
  case 2:
    value = D_801C98B8[index * 11];
    break;
  case 3:
    value = D_801C9E8E[index * 10];
    break;
  }
  return value;
}
