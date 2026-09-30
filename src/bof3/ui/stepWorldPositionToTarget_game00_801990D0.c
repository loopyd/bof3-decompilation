#include "bof3/ui/game00_internal.h"

/* @source 0x801990D0
 * @behavior steps the signed front-end world x at D_801492D8 down by 22 toward
 * its -682 clamp while mirroring the delta into the u16 companion at
 * D_801481D8, then steps the signed world y at D_801492DC toward 512 by 22 in
 * the direction selected by the flag byte at D_80143BB1 (zero = downward,
 * nonzero = upward) while mirroring that delta into the u16 companion at
 * D_801481DC; each pair clamps to its target and the y pair is left untouched
 * once it already equals 512, which is the arrival updateWorldPosition watches
 * for.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801990D0(void) {
  s16* xpos;
  s16* ypos;
  s16  x;
  s16  y;
  u16  m;

  xpos = &D_801492D8;
  x = *xpos;
  if (x >= -0x2A9) {
    m = D_801481D8[0];
    x -= 0x16;
    *xpos = x;
    m -= 0x16;
    D_801481D8[0] = m;
  } else {
    *xpos = -0x2AA;
    D_801481D8[0] = *xpos;
  }
  ypos = &D_801492DC;
  y = *ypos;
  if (y != 0x200) {
    if (D_80143BB1 == 0) {
      if (y > 0x200) {
        m = D_801481DC[0];
        y -= 0x16;
        *ypos = y;
        m -= 0x16;
        D_801481DC[0] = m;
      } else {
        *ypos = 0x200;
        D_801481DC[0] = 0x200;
      }
    } else if (y < 0x200) {
      m = D_801481DC[0];
      y += 0x16;
      *ypos = y;
      m += 0x16;
      D_801481DC[0] = m;
    } else {
      *ypos = 0x200;
      D_801481DC[0] = 0x200;
    }
  }
}
