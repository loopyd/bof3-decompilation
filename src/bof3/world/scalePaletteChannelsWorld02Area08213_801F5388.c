#include "bof3/bof3.h"

/* @source 0x80145988 @kind unknown */
extern u8 paletteStageSerial;

/* @source 0x801F5388
 * @behavior Overlay palette helper: walks all 0x100 entries of the colour
 *           palette window at 0x80034000, splits each halfword into its three
 *           5-bit channels, multiplies every channel by the signed caller
 *           scale, shifts each product right by 7 (so 0x80 is the unity scale)
 *           and packs the three shifted channels back together with bit 15 of
 *           the source word, storing the result into the palette window at
 *           0x80038000. The two windows are the scena16 palette source and
 *           destination bases (0x80033800 / 0x80037800) advanced by 0x800, and
 *           the `lui`/`ori` pairs that build them prove the plain constant
 *           form. The products are signed (`mult` followed by `sra`), while the
 *           channel extracts use logical shifts and one shared `andi 0xffff`
 *           on the loaded halfword. After the loop it stores 1 into the shared
 *           palette-stage byte at 0x80145988 - the byte the exact sibling lift
 *           copyPaletteBlock32World02Area07714_801F529C names paletteStageSerial
 *           - and returns nothing. Its callers inside this payload are
 *           func_801F4A90, func_801F4DC8 and func_801F52D8, which forward the
 *           fade scales 0x40, 0x80 and the work-record byte.
 * @status exact
 * @match 100.00
 * @residual none
 */

void scalePaletteChannelsWorld02Area08213_801F5388(s32 scale) {
  s32 i;
  u16* destination;
  const u16* source;
  u32 color;
  s32 red;
  s32 green;
  s32 blue;

  i = 0;
  destination = PSX_PTR(u16, 0x80038000u);
  source = PSX_PTR(const u16, 0x80034000u);

  do {
    color = *source++;
    red = ((s32)(color & 0x1f) * scale) >> 7;
    green = ((s32)((color & 0xffffu) >> 5 & 0x1fu) * scale) >> 7;
    blue = ((s32)((color & 0xffffu) >> 10 & 0x1fu) * scale) >> 7;
    *destination++ =
        (u16)((color & 0x8000u) | (blue << 10) | (green << 5) | red);
    i++;
  } while (i < 0x100);

  paletteStageSerial = 1;
}
