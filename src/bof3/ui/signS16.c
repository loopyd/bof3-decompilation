#include "bof3/ui/game00_internal.h"

/* @behavior classifies the signed low 16 bits as negative, zero, or positive
 * and returns -1, 0, or 1 respectively.
 * @source 0x801C71AC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 signS16(s32 arg0) {
  s32 v0;
  s32 v1;

  v0 = arg0;
  v1 = v0 << 0x10;
  if (v1 > 0) {
    return 1;
  }
  return v1 >> 0x1F;
}
