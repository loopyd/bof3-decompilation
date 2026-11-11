#include "bof3/bof3.h"

void offsetPaletteChannelsWorld02Area08513_801F3C3C(s32 offset);

/* @source 0x801F3C1C
 * @behavior Thin overlay wrapper: forwards the constant palette-channel offset
 *           -8 to offsetPaletteChannelsWorld02Area08513_801F3C3C; takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperArgWorld02Area08513_801F3C1C(void) {
  offsetPaletteChannelsWorld02Area08513_801F3C3C(-8);
}
