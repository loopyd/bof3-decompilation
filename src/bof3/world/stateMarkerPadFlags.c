#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DDEEC
 * @behavior Use the first pad flag to set the scratch marker and reload work byte 0xA to
 * 8. Otherwise, while the second flag and countdown are nonzero, decrement the
 * countdown and set the marker, including the decrement from 1 to 0. Clear the
 * marker on the other paths.
 * @status exact
 * @match 100.00
 * @residual none
 */

void stateMarkerPadFlags(void)
{
  u8* countdownWork;

  if (D_80145AA8 & 0x4000) {
    countdownWork = SPAD_PTR_SLOT(u8, 0x44u);
    SPAD_REF(u8, 0) = 1;
    countdownWork[0xA] = 8;
    return;
  }

  if (D_80145AA4 & 0x4000) {
    countdownWork = SPAD_PTR_SLOT(u8, 0x44u);
    if (countdownWork[0xA] != 0) {
      countdownWork[0xA] = countdownWork[0xA] - 1;
      SPAD_REF(u8, 0) = 1;
      return;
    }
  }

  SPAD_REF(u8, 0) = 0;
}
