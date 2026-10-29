#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F6B1C[];

/* @source 0x801F59DC
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F6B1C and
 *           publishes it into the shared status halfword D_801490A8; takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusWorld02Area07714_801F59DC(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F6B1C[mode];
}
