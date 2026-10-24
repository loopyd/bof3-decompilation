#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @stores byte 2 at offset 8 and halfword -0xA at offset 6 of the pointer from D_8014639C.record.
 * @source 0x800A0C44
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A0C44(void) {
  *((u8*)D_8014639C.record + 8) = 2;
  D_8014639C.record[3] = -0xA;
}
