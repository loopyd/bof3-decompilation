#include "bof3/bof3.h"

extern u8 D_8014933A;

/* Scratch work-record pointer cell published by the overlay. The original
 * re-reads it once per access group, re-materializing its address
 * (lui $vN,0x1F80 / lw $vN,68($vN)) for each read, so it is named through this
 * target's map instead of the fixed-address accessor. */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */

/* Screen-gradient fill helper, the overlay's own routine at 0x801F5428. Its
 * body splits the packed word it is handed into two five-bit-per-channel
 * colours (top edge from bits 0-14, bottom edge from bits 16-30, each scaled
 * left by three), draws a 0x140 x 0xF0 SetPolyG4 quad through the primitive
 * cursor cell at 0x8014598C with one colour on the top edge and the other on
 * the bottom edge, advances that cursor by 0x24 and returns nothing. Splat
 * binds the address through this target's map row. */
void func_801F5428(s32 color);

/* @source 0x801F4C90
 * @behavior Overlay step handler. It re-reads the scratchpad work pointer cell
 *           0x1F800044, takes that record's byte at +0x09 as a fade level and
 *           packs a 15-bit colour from it: green = level / 5 (signed divide,
 *           magic 0x66666667), blue = green + 6, red = 5 - level / 24
 *           (unsigned divide, magic 0xAAAAAAAB, byte-masked), with bit 28 set.
 *           At level 0 that word is 0x10001805, the constant the neighbouring
 *           step handlers fill with. It clears the shared palette-stage byte at
 *           0x8014933A and hands the colour to the screen-gradient fill helper
 *           at 0x801F5428, which paints the whole 0x140 x 0xF0 screen and
 *           scales each channel left by three, so the ramp reddens down from
 *           (40, 0, 48) while green and blue rise together. It then re-reads
 *           the work cell and advances byte +0x09 by one, and a third cell read
 *           shows that byte: once it has reached 0x79 the function stores 1
 *           into 0x8014933A and advances the record dispatch byte +0x02 by one,
 *           which passes control to the next handler. The 0x18-byte frame holds
 *           the saved return address at 0x10; the function takes no arguments
 *           and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fillScreenRampColorIncrementWorkByte9Set8014933AAdvanceWorkByte2At79World02Area08213_801F4C90(
    void) {
  u8* drawCell;
  u8* incCell;
  u8* gateCell;
  s32 level;
  u32 color;

  drawCell = workRecordCursor1F800044;
  level = drawCell[9];
  color = 0x10000000u | ((u32)(level / 5 + 6) << 10) |
          ((u32)(level / 5) << 5) | (u32)(5 - (u8)((u32)level / 24));
  D_8014933A = 0;
  func_801F5428((s32)color);
  incCell = workRecordCursor1F800044;
  incCell[9]++;
  gateCell = workRecordCursor1F800044;
  if (gateCell[9] >= 0x79) {
    D_8014933A = 1;
    gateCell[2]++;
  }
}
