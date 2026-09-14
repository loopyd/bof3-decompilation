#include "bof3/boot/logo_internal.h"

/*
 * @source 0x801CED48
 * @behavior ramps the LOGO CD mixing record from zero to 0x80 in steps of
 * two, submitting each level to CdMix.
 * @status exact
 * @match 100.00
 * @residual none
 * live comparison is instruction- and byte-exact.
 */
void func_801CED48(void) {
  s32 level;

  for (level = 0; level <= 0x80; level += 2) {
    D_801EB470.val0 = level;
    D_801EB470.val1 = 0;
    D_801EB470.val2 = level;
    D_801EB470.val3 = 0;
    CdMix(&D_801EB470);
  }
}
