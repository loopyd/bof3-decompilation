#include "bof3/bof3.h"

extern u8 D_8014933B;

void func_8015DF18(u16 cue);

/* @source 0x801F5A84
 * @behavior Overlay state handler: clears the overlay state byte at 0x8014933B
 *           and then queues the fixed cue 0x202 through the main-exe cue
 *           dispatcher at 0x8015DF18; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearStateQueueCue202World02Area07714_801F5A84(void) {
  D_8014933B = 0;
  func_8015DF18(0x202);
}
