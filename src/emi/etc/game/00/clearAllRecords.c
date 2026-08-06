#include "internal.h"

/* @behavior loops over 20 local frontend records and resets each one through the
 * shared per-record clear helper.
 * @source 0x8019611C
 */
void clearAllRecords(void) {
  u8 record_index;

  record_index = 0u;
  while (record_index < 0x14u) {
    clearRecord(record_index);
    record_index += 1u;
  }
}
