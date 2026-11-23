#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F5384[];
extern u8 g_ScenarioProgress;

/* @source 0x801F3E04
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the payload halfword run at 0x801F5384 and
 *           publishes it into the shared status halfword D_801490A8; when that
 *           same mode is 5 it also stores 6 into the shared scenario progress
 *           byte 0x80146864. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusTable5384(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F5384[mode];
  if (mode < 0) {
    return;
  }
  if (mode < 5) {
    return;
  }
  if (mode != 5) {
    return;
  }
  g_ScenarioProgress = 6;
}
