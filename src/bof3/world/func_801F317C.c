#include "bof3/world/area00813_internal.h"

/* @behavior Selects area mode 9 when flag bit 7 is set, otherwise mode 5
 * when the secondary state byte equals 3.
 * @source 0x801F317C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801F317C(void)
{
  s32 state;

  if ((D_80146867 & 0x80u) != 0u) {
    g_areaWork->mode = 9u;
  } else {
    state = D_80146866;
    if (state == 3) {
      g_areaWork->mode = 5u;
    }
  }
}
