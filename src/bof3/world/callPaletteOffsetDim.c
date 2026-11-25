#include "bof3/bof3.h"

void mathPaletteChannelClamp(s32 offset);

/* @source 0x801F3C1C
 * @behavior Thin overlay wrapper: forwards the constant palette-channel offset
 *           -8 to mathPaletteChannelClamp; takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callPaletteOffsetDim(void) {
  mathPaletteChannelClamp(-8);
}
