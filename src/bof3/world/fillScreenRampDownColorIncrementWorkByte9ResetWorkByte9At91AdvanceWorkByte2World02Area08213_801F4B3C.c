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

/* @source 0x801F4B3C
 * @behavior Overlay step handler, entry index 3 of the sixteen-pointer step
 *           table at 0x801F57D0. It re-reads the scratchpad work pointer cell
 *           0x1F800044, takes that record's byte at +0x09 as a fade level and
 *           darkens the screen with it: the level is scaled by 27/100 (signed
 *           divide, magic 0x51EB851F) and subtracted from the blue field 0x1F
 *           and from the green field 0x19, while the red field takes
 *           level / 17 (unsigned divide, magic 0xF0F0F0F1, byte-masked), with
 *           bit 28 set. At level 0 that word is 0x10007F20. It clears the
 *           shared palette-stage byte at 0x8014933A and hands the colour to
 *           the screen-gradient fill helper at 0x801F5428, which paints the
 *           whole 0x140 x 0xF0 screen and scales each channel left by three.
 *           It then re-reads the work cell and advances byte +0x09 by one, and
 *           a third cell read shows that byte: once it has reached 0x5B the
 *           function clears byte +0x09 and, through a fourth cell read,
 *           advances the record dispatch byte +0x02 by one, which passes
 *           control to the next handler. The 0x18-byte frame holds the saved
 *           return address at 0x10; the function takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fillScreenRampDownColorIncrementWorkByte9ResetWorkByte9At91AdvanceWorkByte2World02Area08213_801F4B3C(
    void) {
  u8* drawCell;
  u8* incCell;
  u8* gateCell;
  u8* resetCell;
  s32 level;
  u32 color;

  drawCell = workRecordCursor1F800044;
  level = drawCell[9];
  color = 0x10000000u | ((u32)(0x1F - level * 27 / 100) << 10) |
          ((u32)(0x19 - level * 27 / 100) << 5) |
          (u32)(u8)((u32)level / 17);
  D_8014933A = 0;
  func_801F5428((s32)color);
  incCell = workRecordCursor1F800044;
  incCell[9]++;
  gateCell = workRecordCursor1F800044;
  if (gateCell[9] >= 0x5B) {
    gateCell[9] = 0;
    resetCell = workRecordCursor1F800044;
    resetCell[2]++;
  }
}
