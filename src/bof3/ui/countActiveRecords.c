#include "bof3/ui/commu00_internal.h"

/* @source 0x801F02E4
 * @behavior counts non-zero bytes in activeRecordBytes region with stride-8, returns masked count
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 countActiveRecords(void) {
  s32 count = 0;
  s32 v1 = 0;

  do {
    if (((volatile u8 *)activeRecordBytes)[v1] != 0) {
      count++;
    }
    v1 += 8;
  } while (v1 < 0x1E0);

  return (u8)(count & 0xFF);
}
