#include "bof3/bof3.h"

/* @source 0x80145988 @kind unknown */
extern u8 paletteStageSerial;

/* @source 0x801F529C
 * @behavior Copies eight words forward within palette staging RAM at 0x80037800.
 *           Source and destination each use a row stride of 0x200 bytes and a
 *           block stride of 0x20 bytes. Sets the shared palette-stage byte at
 *           0x80145988 to 1 after the copy; returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyPaletteBlock32World02Area07714_801F529C(u32 sourceRow, u32 sourceBlock,
                                               u32 destinationRow, u32 destinationBlock) {
  u32 base;
  u32* source;
  u32* destination;
  s32 i;

  base = (u32)PSX_PTR(u32, 0x80037800u);
  i = 0;
  destination = (u32*)((destinationRow << 9) + ((destinationBlock << 5) + base));
  source = (u32*)((sourceRow << 9) + ((sourceBlock << 5) + base));
  do {
    *destination++ = *source++;
    i++;
  } while (i < 8);
  paletteStageSerial = 1;
}
