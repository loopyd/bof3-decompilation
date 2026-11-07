#include "bof3/bof3.h"

extern u8 D_8014933A;
extern u32 D_80143E6C;

/* Scratch work-record pointer cell published by the overlay. The original
 * re-reads it once per access group, re-materializing its address
 * (lui $vN,0x1F80 / lw $vN,68($vN)) for each read, so it is named through this
 * target's map instead of the fixed-address accessor. */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */

/* Quad-band drawer, the overlay's own routine at 0x801F46C8. Its body pushes
 * the current matrix, projects the scratch translation vector at 0x1F800014
 * through RotTrans and the scratch rotation matrix at 0x1F80001C through
 * RotMatrix, and then walks one row per index from its first argument up to
 * its second, drawing two translucent POLY_G4 quads per row: each quad's
 * offsets come from the overlay's signed-byte row table at 0x801F578A scaled
 * by the fourth argument, and the third argument reaches func_80179F98
 * together with the projected matrix. Every row is queued through
 * func_8014E5A0, and the routine closes with PopMatrix. Splat binds the
 * address through this target's generated asm label. */
void func_801F46C8(s32 first_row, s32 row_limit, s32 matrix_seed, s32 row_scale);

/* Screen-gradient fill helper, the overlay's own routine at 0x801F5428. It
 * splits the packed word it is handed into two five-bit-per-channel colours
 * (top edge from bits 0-14, bottom edge from bits 16-30, each scaled left by
 * three), draws a 0x140 x 0xF0 SetPolyG4 quad through the primitive cursor
 * cell at 0x8014598C with one colour on the top edge and the other on the
 * bottom edge, advances that cursor by 0x24 and returns nothing. Splat binds
 * the address through this target's map row. */
void func_801F5428(s32 color);

/* @source 0x801F4F88
 * @behavior Overlay step handler, entry index 9 of the sixteen-pointer step
 *           table at 0x801F57D0, so the byte +0x02 advance below passes
 *           control to the following entry at 0x801F5068. It seeds the
 *           overlay's scratch geometry first: the halfwords at 0x1F800014,
 *           0x1F800016 and 0x1F800018 take -0x2E40, -0x3240 and -0x3E0, which
 *           is the translation vector the quad-band drawer at 0x801F46C8
 *           projects through RotTrans; the halfwords at 0x1F80001C and
 *           0x1F80001E are cleared and 0x1F800020 takes the shared frame
 *           counter at 0x80143E6C shifted left by five, which is the first row
 *           of the rotation matrix that drawer hands to RotMatrix. It then
 *           draws one translucent quad band through that drawer, from row
 *           4 x the work record byte +0x09 up to 0x1F, passing 0 as the matrix
 *           seed and 0x18 as the per-row offset scale. Next it clears the
 *           shared palette-stage byte at 0x8014933A and paints the whole
 *           0x140 x 0xF0 screen with the constant colour word 0x10001805
 *           through the gradient fill helper at 0x801F5428. It then re-reads
 *           the scratchpad work pointer cell 0x1F800044 and increments that
 *           record's +0x09 byte, and re-reads the cell once more to test the
 *           incremented byte against 8 as an unsigned bound: only once that
 *           byte has reached 8 does the record's +0x02 dispatch byte advance
 *           by one. The function has a 0x18-byte frame holding the saved
 *           return address at 0x10, takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScratchVectorsDrawQuadBandFillScreenGradientIncrementWorkByte9At8World02Area08213_801F4F88(
    void) {
  u8* work;
  u8* tick;
  u8* step;

  SPAD_REF(s16, 0x14u) = -0x2E40;
  SPAD_REF(s16, 0x16u) = -0x3240;
  SPAD_REF(s16, 0x18u) = -0x3E0;
  work = workRecordCursor1F800044;
  SPAD_REF(s16, 0x1Cu) = 0;
  SPAD_REF(s16, 0x1Eu) = 0;
  SPAD_REF(s16, 0x20u) = (s16)(D_80143E6C << 5);
  func_801F46C8(work[9] << 2, 0x1F, 0, 0x18);
  D_8014933A = 0;
  func_801F5428(0x10001805);
  tick = workRecordCursor1F800044;
  tick[9]++;
  step = workRecordCursor1F800044;
  if (step[9] >= 8u) {
    step[2]++;
  }
}
