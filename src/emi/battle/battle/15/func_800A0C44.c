#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores byte 2 at offset 8 and halfword -0xA at offset 6 of the pointer from D_801463A0.
 * @source 0x800A0C44
 */
void func_800A0C44(void) {
  *((u8*)D_801463A0 + 8) = 2;
  D_801463A0[3] = -0xA;
}
