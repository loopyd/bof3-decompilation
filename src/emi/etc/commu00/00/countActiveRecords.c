#include "internal.h"

/* activeRecordBytes: 480-byte region, accessed with stride-8 byte offsets */
extern volatile u8 activeRecordBytes[];

/* @source 0x801F02E4
 * @behavior counts non-zero bytes in activeRecordBytes region with stride-8, returns masked count
 */
u8 countActiveRecords(void) {
  s32 count = 0;
  s32 v1 = 0;

  do {
    if (activeRecordBytes[v1] != 0) {
      count++;
    }
    v1 += 8;
  } while (v1 < 0x1E0);

  return (u8)(count & 0xFF);
}
