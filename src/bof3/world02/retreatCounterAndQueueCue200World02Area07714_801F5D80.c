#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

void func_8015DF18(u16 cue);

/* @source 0x801F5D80
 * @behavior Overlay counter step: reads the shared halfword counter at
 *           0x8014932A, marks the shared area status byte at 0x80149333 as 2,
 *           retreats that counter by 0x14, and then queues the fixed cue 0x200
 *           through the main-exe cue dispatcher at 0x8015DF18; takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounterAndQueueCue200World02Area07714_801F5D80(void) {
  u16 count;

  count = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count - 0x14);
  func_8015DF18(0x200);
}
