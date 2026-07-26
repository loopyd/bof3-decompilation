#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @increments the global counter at D_801462E4 if D_801485E0 equals 0xA3.
 * @source 0x800A60A8
 */
void func_800A60A8(void) {
  if (D_801485E0 == 0xA3) {
    (*D_801462E4)++;
  }
}
