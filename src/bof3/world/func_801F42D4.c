#include "bof3/world/area00813_internal.h"

/* @source 0x801F42D4
 * @behavior selects a halfword from the local mode table, stores it to the
 * shared area value, and maps modes zero and one to scenario flags one and
 * zero respectively.
 */
void func_801F42D4(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F53F0[mode];
  switch (mode) {
  case 0:
    g_ScenarioProgress = 1;
    break;
  case 1:
    g_ScenarioProgress = 0;
    break;
  default:
    break;
  }
}
