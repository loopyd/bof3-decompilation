#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F539C[];
extern u8 g_ScenarioProgress;

/* @source 0x801F3E7C
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F539C and
 *           publishes it into the shared status halfword D_801490A8; while the
 *           same mode is 0 or 1 it also stores 5 into the shared scenario
 *           progress byte 0x80146864. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFromTable539cAndProgress5World01Area06714_801F3E7C(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F539C[mode];
  if (mode >= 2) {
    return;
  }
  if (mode < 0) {
    return;
  }
  g_ScenarioProgress = 5;
}
