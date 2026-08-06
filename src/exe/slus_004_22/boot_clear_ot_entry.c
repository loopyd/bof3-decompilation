#include "internal.h"

/* @behavior clears one ordering table and resets its four boot render flags.
 * @source 0x8014AE9C
 */
void boot_clear_ot_entry(u8* work) {
  ClearOTagR((u_long*)(work + 0x70), 8);
  work[0x2c] = 1;
  work[0x2d] = 0;
  work[0x2e] = 0;
  work[0x2f] = 0;
}
