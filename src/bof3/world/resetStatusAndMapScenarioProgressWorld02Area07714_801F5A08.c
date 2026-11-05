#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 g_ScenarioProgress;

/* @source 0x801F5A08
 * @behavior Reads the signed shared mode byte at 0x801490C7, writes 0xFFFF
 *           to the shared status halfword at 0x801490A8, and maps mode 0 to
 *           scenario progress 1 and mode 1 to scenario progress 0. Other
 *           modes leave scenario progress unchanged. Takes no arguments
 *           and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetStatusAndMapScenarioProgressWorld02Area07714_801F5A08(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = 0xFFFF;
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
