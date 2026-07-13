#include "internal.h"

/* @behavior clears one ordering table and resets its four boot render flags.
 * @source 0x8014ae9c func_8014ae9c
 */
void func_8014ae9c(u8* work) {
  func_8017b8d4(work + 0x70, 8);
  work[0x2c] = 1;
  work[0x2d] = 0;
  work[0x2e] = 0;
  work[0x2f] = 0;
}
