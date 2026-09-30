#include "bof3/world/area03004_internal.h"

/* @source 0x801E1C7C
 * @behavior draws the AREA030 extra icon when the D_801E3214 flag byte is
 * nonzero and the shared frame counter D_80143E6C has bit 3 set: submits
 * texture-page draw mode 3/1 through submitTpageDrawMode and appends sprite
 * 0x47 at (0x1A, 0x5C) through func_801E0DCC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E1C7C(void) {
  if (D_801E3214 != 0) {
    if ((D_80143E6C & 8) != 0) {
      submitTpageDrawMode(3, 1);
      func_801E0DCC(0x47, 1, 0x1A, 0x5C);
    }
  }
}
