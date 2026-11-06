#include "bof3/bof3.h"

/* One labelled card cell of this overlay: a 0x2A by 0x14 panel, the 48 by 24
 * sprite selected by glyph from the four-byte sheet entries at 0x801DAC60
 * (glyphs 9 and 10), and the label run selected by label from the strings
 * HIGH, LOW and SET at 0x801DAC54. Glyph 0 draws sprite 10 with the bright
 * four-token label, glyph 1 sprite 9 with the bright three-token label and
 * glyph 2 sprite 10 with the shaded three-token label. */
extern void func_801D2AC4(s16 x, s16 y, u8 label, u8 glyph);

/* @source 0x801D2908
 * @behavior Draws the three labelled card cells of one row through
 * func_801D2AC4 at the left edges x, x + 0x30 and x + 0x60, handing each cell
 * its own index (0, 1, 2) as the label selector so the row carries the HIGH,
 * LOW and SET labels of the rodata run at 0x801DAC54 in that order. The
 * per-cell sprite glyph comes from kind: the 0xFF sentinel draws glyph 0 on
 * all three cells, 0 draws glyphs 1, 2 and 0, 1 draws 2, 1 and 0, 2 draws 0, 0
 * and 2, and 3 draws 2, 2 and 0; any other kind value draws nothing. Both
 * coordinates reach the callee narrowed to s16, the row coordinate through one
 * conversion shared by the three calls of a kind. Nothing is written and no
 * value is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawThreeCardRowStates(s32 x, s32 y, u8 kind) {
  switch (kind) {
    case 0xFF:
      func_801D2AC4((s16)x, (s16)y, 0, 0);
      func_801D2AC4((s16)(x + 0x30), (s16)y, 1, 0);
      func_801D2AC4((s16)(x + 0x60), (s16)y, 2, 0);
      break;
    case 0:
      func_801D2AC4((s16)x, (s16)y, 0, 1);
      func_801D2AC4((s16)(x + 0x30), (s16)y, 1, 2);
      func_801D2AC4((s16)(x + 0x60), (s16)y, 2, 0);
      break;
    case 1:
      func_801D2AC4((s16)x, (s16)y, 0, 2);
      func_801D2AC4((s16)(x + 0x30), (s16)y, 1, 1);
      func_801D2AC4((s16)(x + 0x60), (s16)y, 2, 0);
      break;
    case 2:
      func_801D2AC4((s16)x, (s16)y, 0, 0);
      func_801D2AC4((s16)(x + 0x30), (s16)y, 1, 0);
      func_801D2AC4((s16)(x + 0x60), (s16)y, 2, 2);
      break;
    case 3:
      func_801D2AC4((s16)x, (s16)y, 0, 2);
      func_801D2AC4((s16)(x + 0x30), (s16)y, 1, 2);
      func_801D2AC4((s16)(x + 0x60), (s16)y, 2, 0);
      break;
  }
}
