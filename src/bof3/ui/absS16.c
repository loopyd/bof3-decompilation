#include "bof3/ui/game00_internal.h"

/* @behavior returns the absolute value of one signed 16-bit argument.
 * @source 0x801C7188
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s16 absS16(s16 arg0) {
  if ((s32)arg0 < 0)
    return -arg0;
  return arg0;
}
