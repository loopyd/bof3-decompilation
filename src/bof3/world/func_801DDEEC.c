#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DDEEC
 * @behavior Arms or expires the AREA030 hold marker: while the first shared
 * pad flag is set it latches the scratchpad byte at 0x1F800000 to 1 and
 * reloads the work-record timer at +0xA to 8; otherwise the second shared pad
 * flag counts that timer down until it reaches zero, and any unarmed path
 * clears the marker.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 36/36 instructions, 144 bytes, live byte match.
 */

void func_801DDEEC(void)
{
  u8* work;

  if (D_80145AA8 & 0x4000) {
    work = SPAD_PTR_SLOT(u8, 0x44u);
    SPAD_REF(u8, 0) = 1;
    work[0xA] = 8;
    return;
  }

  if (D_80145AA4 & 0x4000) {
    work = SPAD_PTR_SLOT(u8, 0x44u);
    if (work[0xA] != 0) {
      work[0xA] = work[0xA] - 1;
      SPAD_REF(u8, 0) = 1;
      return;
    }
  }

  SPAD_REF(u8, 0) = 0;
}
