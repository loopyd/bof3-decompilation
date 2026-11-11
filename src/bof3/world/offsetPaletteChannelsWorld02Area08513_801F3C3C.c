#include "bof3/bof3.h"

/* @source 0x80145988 @kind unknown */
extern u8 paletteStageSerial;

/* @source 0x801F3C3C
 * @behavior Overlay palette helper: walks the 0x100 halfwords from the palette
 *           source window at 0x80034400 up to the exclusive destination limit
 *           0x80038600, storing into the mirror window at 0x80038400. Every
 *           source halfword is split into its three 5-bit channels (red from
 *           the low bits, then green, then blue), the halfword is masked with
 *           0xFFFF once for the green and blue extracts so those two shift
 *           logically, and the three channels are packed back with bit 15 of a
 *           re-read source halfword. Each channel is skipped when it is zero;
 *           otherwise the signed caller offset is added to it and the sum is
 *           clamped into 0..0x1F with a signed `bgtz` zero test and a signed
 *           `slti 0x20` ceiling test, so a full-bright channel stays at 0x1F
 *           instead of wrapping. The destination cursor is compared signed
 *           (`slt`) with the limit constant and the source cursor advances in
 *           that branch's delay slot. After the loop it stores 1 into the
 *           shared palette-stage byte at 0x80145988, the byte the exact
 *           sibling lifts scalePaletteChannelsWorld02Area08213_801F5388 and
 *           shiftPaletteBlocksChannelsWorld03Area14313_801F3E60 name
 *           paletteStageSerial, and returns nothing. Leaf: no frame, `jr $ra`
 *           plus `nop`. Its only caller inside this payload is
 *           invokeHelperArgWorld02Area08513_801F3C1C, which forwards the
 *           constant offset -8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void offsetPaletteChannelsWorld02Area08513_801F3C3C(s32 offset) {
  u16* destination;
  const u16* source;
  u16* limit;
  s32 red;
  s32 green;
  s32 blue;

  destination = PSX_PTR(u16, 0x80038400u);
  source = PSX_PTR(const u16, 0x80034400u);
  limit = PSX_PTR(u16, 0x80038600u);

  do {
    red = *source & 0x1F;
    green = ((*source & 0xFFFFu) >> 5) & 0x1F;
    blue = ((*source & 0xFFFFu) >> 10) & 0x1F;

    if (red != 0) {
      red += offset;
      if (red <= 0) {
        red = 0;
      }
      if (red > 0x1F) {
        red = 0x1F;
      }
    }

    if (green != 0) {
      green += offset;
      if (green <= 0) {
        green = 0;
      }
      if (green > 0x1F) {
        green = 0x1F;
      }
    }

    if (blue != 0) {
      blue += offset;
      if (blue <= 0) {
        blue = 0;
      }
      if (blue > 0x1F) {
        blue = 0x1F;
      }
    }

    *destination =
        (u16)((*source & 0x8000u) | (blue << 10) | (green << 5) | red);
    destination++;
    source++;
  } while ((s32)destination < (s32)limit);

  paletteStageSerial = 1;
}
