#include "bof3/bof3.h"

/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;
/* @source 0x80146871 @kind unknown */
extern u8 D_80146871;
/* @source 0x80145E9A @kind unknown */
extern u8 D_80145E9A;
/* @source 0x80145E9B @kind unknown */
extern u8 D_80145E9B;
/* @source 0x80143E6C @kind unknown */
extern u32 D_80143E6C;

void func_801F5AA0(s32 arg0, s32 arg1, s32 arg2);
void func_801F595C(s32 arg0, s32 arg1);

/* @source 0x801F5B90
 * @behavior Draws the AREA121 meter widget unless byte 0x8014832E holds none of
 * bits 0x1B: a 0x50-by-0x20 shaded sprite at (0xDC, 0x10) through the fixed-size
 * emitter at 0x801F5AA0, then a 64-pixel textured bar at (0xE4, 0x1D) through the
 * bar emitter at 0x801F595C, whose two arguments are a width and a palette index.
 * All three arms end in one shared bar call that each arm prepares. The arm taken
 * when bit 0x40 of byte 0x80146871 is set draws the sprite at shade level 0, the
 * palette-0 base bar, and then a full 0x40 bar with palette 1. The arm taken when
 * byte 0x80145E9A is nonzero draws the sprite at shade level 1 or 2 selected by
 * bit 2 of the shared frame counter 0x80143E6C and only the 0x40 palette-0 bar.
 * The remaining arm draws the sprite at shade level 0, the palette-0 base bar,
 * and then a palette-1 bar whose width is (0x40 - byte 0x80145E9B) & 0xFF, so the
 * highlight recedes as that byte grows. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The width staircase in the last arm is the shape the original build needed: the
 * constant 0x40 is published into the width local before the byte is subtracted, so
 * both the subtraction and the `u8` truncation land in the call's first argument
 * register. Folding the sum into one expression measured three register differences
 * at unchanged size. The shade arms write the same call twice with the differing
 * constant, which merged into the one emitter call the original keeps. */
void drawShadedMeterFrameAndDepletingFillWorld03Area12114_801F5B90(void) {
  s32 width;
  s32 palette;

  if ((D_8014832E & 0x1B) == 0) {
    return;
  }
  if ((D_80146871 & 0x40) == 0) {
    if ((D_80145E9A == 0)) {
      func_801F5AA0(0xDC, 0x10, 0);
      func_801F595C(0x40, 0);
      palette = 1;
      width = 0x40;
      width -= D_80145E9B;
      width = (u8)width;
    } else {
      if (D_80143E6C & 2) {
        func_801F5AA0(0xDC, 0x10, 1);
      } else {
        func_801F5AA0(0xDC, 0x10, 2);
      }
      width = 0x40;
      palette = 0;
    }
  } else {
    func_801F5AA0(0xDC, 0x10, 0);
    func_801F595C(0x40, 0);
    width = 0x40;
    palette = 1;
  }
  func_801F595C(width, palette);
}
