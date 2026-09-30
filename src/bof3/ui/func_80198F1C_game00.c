#include "bof3/ui/game00_internal.h"

/* @source 0x80198F1C
 * @behavior gates the pad state word at D_80145AA4 with the mask at
 * D_80145ABA: while the two share no bit it publishes the world-facing
 * direction byte at D_80143BB1 from the signed front-end world y at
 * D_801492DC (0 when y is above 512, 1 when y is below 512, untouched at
 * exactly 512) and advances the front-end sub-state word at D_80143B92 by one;
 * otherwise it steps the signed world x at D_801492D8 down by 22 while it stays
 * at or above -681 (gate bit 0x4000) or up by 22 while it stays below -455
 * (bit 0x1000), and the signed world y at D_801492DC up by 22 while it stays
 * below 853 (bit 0x2000) or down by 22 while it stays at or above 171 (bit
 * 0x8000), mirroring every accepted delta into the u16 companion at
 * D_801481D8 (x) / D_801481DC (y); each gate bit re-reads the pad state word
 * and each axis block is skipped once its clamp is reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_80198F1C(void) {
  s16* pos;
  u16* counter;
  s16  v;
  u16  m;

  if ((D_80145AA4 & D_80145ABA) == 0) {
    pos = &D_801492DC;
    v = *pos;
    if (v > 512) {
      D_80143BB1 = 0;
    } else if (v < 512) {
      D_80143BB1 = 1;
    }
    counter = &D_80143B92;
    *counter = *counter + 1;
    return;
  }
  if (D_80145AA4 & 0x4000) {
    pos = &D_801492D8;
    v = *pos;
    if (v >= -681) {
      m = D_801481D8[0];
      v -= 22;
      *pos = v;
      m -= 22;
      D_801481D8[0] = m;
    }
  }
  if (D_80145AA4 & 0x1000) {
    pos = &D_801492D8;
    v = *pos;
    if (v < -455) {
      m = D_801481D8[0];
      v += 22;
      *pos = v;
      m += 22;
      D_801481D8[0] = m;
    }
  }
  if (D_80145AA4 & 0x2000) {
    pos = &D_801492DC;
    v = *pos;
    if (v < 853) {
      m = D_801481DC[0];
      v += 22;
      *pos = v;
      m += 22;
      D_801481DC[0] = m;
    }
  }
  if (D_80145AA4 & 0x8000) {
    pos = &D_801492DC;
    v = *pos;
    if (v >= 171) {
      m = D_801481DC[0];
      v -= 22;
      *pos = v;
      m -= 22;
      D_801481DC[0] = m;
    }
  }
}
