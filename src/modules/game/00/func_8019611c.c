#include "internal.h"

/* @behavior loops over 20 local frontend records and resets each one through the
 * shared per-record clear helper.
 * @source 0x8019611c FUN_8019611c
 */
void func_8019611c(void) {
  u8 record_index;

  record_index = 0u;
  while (record_index < 0x14u) {
    func_801960c0(record_index);
    record_index += 1u;
  }
}
