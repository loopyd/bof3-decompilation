#include "bof3/ui/shop00_internal.h"

/* @source 0x801E4D4C
 * @behavior byte-list membership test: returns 1 when the byte argument
 *           appears in the ten-byte list of any of the seven 164-byte records
 *           at D_801449E2, or in the 128-byte list at D_8014546C; else 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801E4D4C(u8 arg0) {
  u8 i;
  u8 j;
  u8* p;

  for (i = 0; i < 7; i++) {
    p = D_801449E2 + i * 164;
    for (j = 0; j < 10; j++) {
      if (*p == arg0) {
        return 1;
      }
      p++;
    }
  }

  p = D_8014546C;
  for (j = 0; j < 128; j++) {
    if (*p == arg0) {
      return 1;
    }
    p++;
  }

  return 0;
}
