#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801DDD4C
 * @behavior When selected value byte D_801E31F8 differs from saved previous
 * selection byte D_801E31FC, calls func_8015DF18 with cue 0x203 for
 * value 0, 0x202 for value 1 or 0x205 for value 2. Other values stay
 * silent. Does not write the saved selection byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void audioCueChangedValue(void) {
  u8 selectedValue;

  selectedValue = D_801E31F8;
  if (selectedValue != D_801E31FC) {
    switch (selectedValue) {
    case 0:
      func_8015DF18(0x203);
      break;
    case 1:
      func_8015DF18(0x202);
      break;
    case 2:
      func_8015DF18(0x205);
      break;
    }
  }
}
