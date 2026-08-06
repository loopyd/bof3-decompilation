#include "internal.h"

/* commu00_active_record_bytes: 480-byte region, accessed with stride-8 byte offsets */
extern volatile u8 commu00_active_record_bytes[];

/* @source 0x801F02E4
 * @behavior counts non-zero bytes in commu00_active_record_bytes region with stride-8, returns masked count
 */
u8 commu00_count_active_records(void) {
  s32 count = 0;
  s32 v1 = 0;

  do {
    if (commu00_active_record_bytes[v1] != 0) {
      count++;
    }
    v1 += 8;
  } while (v1 < 0x1E0);

  return (u8)(count & 0xFF);
}
