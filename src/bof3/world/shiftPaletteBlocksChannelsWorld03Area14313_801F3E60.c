#include "bof3/bof3.h"

/* @source 0x80145988 @kind unknown */
extern u8 paletteStageSerial;

/* @source 0x801F3E60
 * @behavior Overlay palette helper: walks 11 consecutive 0x100-halfword
 *           palette blocks, reading the block window at 0x80033E00 and storing
 *           into the mirror window at 0x80037E00. Every source halfword is
 *           split into its three 5-bit channels (red from the low bits, then
 *           green, then blue), each channel is shifted right by the caller
 *           count with an arithmetic shift (`srav` on the variable), and the
 *           three shifted channels are packed back into bits 14..0 beside bit
 *           15 of the source halfword, which is passed through with
 *           `andi 0x8000`. The 0xFFFF mask on the loaded halfword feeds the
 *           green and blue extracts, so those two use logical `srl` while the
 *           caller-count shifts stay signed. Each window cursor starts at its
 *           window base on every outer iteration and the two base pointers
 *           advance 0x200 bytes per block, while the block counter starts at 3
 *           and runs while it is below 0xE. The inner loop counts its 0x100
 *           entries in a register compared with `slti 0x100`. After the last
 *           block it stores 1 into the shared palette-stage byte at 0x80145988
 *           and returns nothing. No function in this payload calls it, so the
 *           caller that supplies the shift count is outside this overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void shiftPaletteBlocksChannelsWorld03Area14313_801F3E60(s32 shift) {
  s32 block;
  s32 i;
  u16* destination;
  const u16* source;
  u16* out;
  const u16* in;
  u32 color;
  s32 red;
  s32 green;
  s32 blue;

  block = 3;
  destination = PSX_PTR(u16, 0x80037E00u);
  source = PSX_PTR(const u16, 0x80033E00u);

  do {
    i = 0;
    out = destination;
    in = source;

    do {
      color = *in++;
      red = (s32)(color & 0x1f) >> shift;
      green = (s32)((color & 0xffffu) >> 5 & 0x1fu) >> shift;
      blue = (s32)((color & 0xffffu) >> 10 & 0x1fu) >> shift;
      *out++ =
          (u16)((color & 0x8000u) | (blue << 10) | (green << 5) | red);
      i++;
    } while (i < 0x100);

    destination += 0x100;
    block++;
    source += 0x100;
  } while (block < 0xE);

  paletteStageSerial = 1;
}
