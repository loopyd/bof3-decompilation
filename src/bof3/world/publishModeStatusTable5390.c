#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F5390[];

/* @source 0x801F3E50
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F5390 and
 *           publishes it into the shared status halfword D_801490A8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusTable5390(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F5390[mode];
}
