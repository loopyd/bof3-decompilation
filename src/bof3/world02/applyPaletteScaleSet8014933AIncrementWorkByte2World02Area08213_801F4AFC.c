#include "bof3/bof3.h"

extern u8 D_8014933A;

void func_801F5388(s32 arg0);

/* @source 0x801F4AFC
 * @behavior Overlay step handler: applies the palette-scale helper at
 *           0x801F5388 to the 0x100-entry colour-palette window with the scale
 *           0x80 (that helper divides each 5-bit channel of the source window
 *           by 2^7 and preserves bit 15, so 0x80 is the unity scale), stores 1
 *           into the shared byte at 0x8014933A, then advances the scratch work
 *           record's +0x02 byte by one; it takes no arguments and returns
 *           nothing. The palette-window addresses and the 0x100-entry count
 *           come from the helper's own body at 0x801F5388, whose window pair
 *           matches the ones the scenario lifts drive through
 *           SCENA16_PALETTE_SRC / SCENA16_PALETTE_DST; the shared byte
 *           0x8014933A is the flag the sibling handler func_801F4A90 clears
 *           while scaling the same window by 0x40.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyPaletteScaleSet8014933AIncrementWorkByte2World02Area08213_801F4AFC(
    void) {
  u8* work;

  func_801F5388(0x80);
  work = SPAD_PTR_SLOT(u8, 0x44);
  D_8014933A = 1;
  work[2]++;
}
