#include "bof3/bof3.h"

/* @source 0x80145988 @kind unknown */
extern u8 paletteStageSerial;

/* @source 0x80117B60
 * @behavior Overlay palette-row helper. Walks the sixteen halfwords of the
 *           palette row at 0x80033F40 and writes the scaled row to 0x80037F40,
 *           the same 0x4000 separation the palette-block scaler
 *           scalePaletteBlocksWorld04Area19813_801F2D28 carries between the
 *           palette blocks at 0x80034000 and 0x80038000. One pointer walks the
 *           destination row and each source halfword is read 0x2000 elements
 *           (0x4000 bytes) below it. Every halfword is split into its three
 *           5-bit fields; the low field (red) and the bits 10-14 field (blue)
 *           are multiplied by the signed caller factor and divided by 0x80, so
 *           0x80 is the unity scale, while the bits 5-9 field (green) is
 *           carried through unscaled. The products are signed: `mult` plus
 *           `bgez`/`addiu 0x7f`/`sra 7` is truncating signed division, not a
 *           plain shift. The three fields are packed back under bit 15 of the
 *           same source halfword, and that halfword is masked with 0xFFFF once
 *           for the two shifted extracts. The loop counter is compared signed
 *           (`slti 0x10`, sixteen entries) and the destination pointer advances
 *           in that branch's delay slot. After the row it stores 1 into the
 *           shared palette-stage byte at 0x80145988, the byte the sibling lifts
 *           name paletteStageSerial. Leaf: no frame, `jr $ra` plus `nop`. Both
 *           callers inside this payload scale one row: scalePaletteRedAndBlueToHalfWhenFlag3ESetThenResetWorkAreaAndAdvanceWorkByte2World01Area04913_80117A74 passes the
 *           constant 0x40, and func_80117ACC passes 0x80 minus half of the work
 *           byte at +9, so the row fades from unity towards black as that byte
 *           climbs to 0x80. Takes the factor as its only argument and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void scalePaletteRow16KeepGreenUnscaledPublishSerialWorld01Area04913_80117B60(
    s32 scale) {
  u16* row;
  u32 color;
  s32 red;
  s32 green;
  s32 blue;
  s32 i;

  i = 0;
  row = PSX_PTR(u16, 0x80037F40u);
  do {
    color = row[-0x2000];
    red = ((s32)(color & 0x1Fu) * scale) / 0x80;
    green = (s32)(((color & 0xFFFFu) >> 5) & 0x1Fu);
    blue = ((s32)(((color & 0xFFFFu) >> 10) & 0x1Fu) * scale) / 0x80;
    *row =
        (u16)((color & 0x8000u) | (blue << 10) | (green << 5) | red);
    i++;
    row++;
  } while (i < 0x10);

  paletteStageSerial = 1;
}
