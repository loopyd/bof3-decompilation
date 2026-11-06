#include "bof3/bof3.h"

/* @source 0x80145988 @kind unknown */
extern u8 paletteStageSerial;

/* @source 0x801F2D28
 * @behavior Overlay palette helper: walks both 0x100-entry colour palette
 *           blocks of the source window at 0x80034000, splits each halfword
 *           into its three 5-bit channels, multiplies every channel by the
 *           caller scale and divides each product by 0x80, so 0x80 is the
 *           unity scale. The products are signed: `mult` plus
 *           `bgez`/`addiu 0x7f`/`sra 7` is truncating signed division, not a
 *           plain shift. The three truncated channels are packed back into the
 *           destination window at 0x80038000 in blue-high order, and a word
 *           that packs to zero while its source halfword is nonzero stores
 *           0x8000 so a fully faded but present palette entry stays flagged
 *           instead of becoming a zero word. The packed accumulator reuses the
 *           red channel's local, which is what leaves the whole word in that
 *           one register (`or` into it) across the zero test and both stores.
 *           The two cursors compare signed (`slt`), so the loop bound is a
 *           signed view of the cursors; the source pointer of the near-identical
 *           lift clampPaletteChannels records the same `slt` evidence. After
 *           both blocks it stores 1 into the shared palette-stage byte at
 *           0x80145988, the byte the exact sibling lifts name
 *           paletteStageSerial, and returns nothing. Its callers inside this
 *           payload are func_801F2C48, which passes the scale 0, and
 *           func_801F2CB8, which passes work-record byte +9 as the scale.
 * @status exact
 * @match 100.00
 * @residual none
 */
void scalePaletteBlocksWorld04Area19813_801F2D28(s32 scale) {
  s32 block;
  u16* destination;
  const u16* source;
  u16* out;
  const u16* in;
  u16* limit;
  u32 color;
  u32 packed;
  s32 green;
  s32 blue;

  block = 0;
  destination = PSX_PTR(u16, 0x80038000u);
  source = PSX_PTR(const u16, 0x80034000u);

  do {
    out = destination;
    in = source;
    limit = destination + 0x100;
    do {
      color = *in;
      /* packed starts on the low channel and accumulates the whole word. */
      packed = ((s32)(color & 0x1f) * scale) / 0x80;
      green = ((s32)((color & 0xffffu) >> 5 & 0x1fu) * scale) / 0x80;
      blue = ((s32)((color & 0xffffu) >> 10 & 0x1fu) * scale) / 0x80;
      packed |= (blue << 10) | (green << 5);
      if (packed == 0 && (color & 0xffffu) != 0) {
        packed = 0x8000;
      }
      *out++ = (u16)packed;
      in++;
    } while ((s32)out < (s32)limit);

    destination += 0x100;
    block++;
    source += 0x100;
  } while (block < 2);

  paletteStageSerial = 1;
}
