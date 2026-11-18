#include "bof3/bof3.h"

extern volatile u16 D_8014932C;
extern volatile u8 D_80149333;

/* @source 0x801F2C04
 * @behavior Overlay channel step and the advance mirror of the adjacent
 * 0x801F2C2C twin, which retreats the same cells: it reads the shared 16-bit
 * channel at 0x8014932C, publishes mode 2 at the shared status byte 0x80149333
 * and then writes the channel back advanced by 0xA00 (the twin subtracts it).
 * Takes no arguments and returns nothing; no frame is built.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceChannel932cByA00World00Area01913_801F2C04(void) {
  u16 channel;

  channel = D_8014932C;
  D_80149333 = 2;
  D_8014932C = (u16)(channel + 0xA00);
}
