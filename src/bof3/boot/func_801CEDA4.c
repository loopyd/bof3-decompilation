#include "bof3/boot/logo_internal.h"

/*
 * @source 0x801CEDA4
 * @behavior ramps the LOGO CD mixing record from 0x80 down to zero in steps of
 * two, submitting each level to CdMix.
 * @status exact
 * @match 100.00
 * @residual none
 * live comparison is instruction- and byte-exact.
 */
void func_801CEDA4(void) {
  s32 level;

  for (level = 0x80; level >= 0; level -= 2) {
    D_801EB470.val0 = level;
    D_801EB470.val1 = 0;
    D_801EB470.val2 = level;
    D_801EB470.val3 = 0;
    CdMix(&D_801EB470);
  }
}
