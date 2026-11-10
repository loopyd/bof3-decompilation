#include "bof3/bof3.h"

extern u32 D_80143E6C;
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */
extern s8 D_801F5764[]; /* @source 0x801F5764 @kind data */
extern s8 D_801F5765[]; /* @source 0x801F5765 @kind data */
extern s16 D_1F800018; /* @source 0x1F800018 @kind unknown */
extern void func_801F46C8(s32 first_row, s32 row_limit, s32 matrix_seed, s32 row_scale); /* @source 0x801F46C8 */
extern void func_801F4430(void); /* @source 0x801F4430 */
extern void func_8015DF18(u32 cue); /* @source 0x8015DF18 */

/* @source 0x801F3E08
 * @behavior Handler 4 of the work-byte-2 table at 0x801F576C. Seeds scratch
 *           translation (-0x1A40, -0x14C0, 0x160) and rotation (0x620, 0),
 *           then draws four quad bands from work byte +0x09 times four up to
 *           row 0x1F, with seeds starting at 0xE00 and stepping by 0x400.
 *           Rotation element 2 takes the frame counter shifted by five plus
 *           the pass index shifted by ten. At work count three or greater,
 *           seeds translation Z to 0x3A0 and rotation to (0x400, 0), then
 *           draws four corner bands using the signed pairs at 0x801F5764/5,
 *           rows zero through 0x1D and scale 0x10. Always draws the work
 *           record through 0x801F4430 and increments byte +0x09; when the
 *           reloaded unsigned byte reaches eight, advances byte +0x02 and
 *           queues cue 0x200. Takes no arguments and returns nothing.
 * @status partial
 * @match 97.66
 * @residual First-loop argument setup and induction update scheduling differ.
 */
void drawRotatingQuadBandsAndCornersAdvanceWorkByte2QueueCue200At8World02Area08213_801F3E08(void) {
  s16* vec;
  s16* matrix;
  u8* tick;
  u8* step;
  s32 i;
  s32 k;
  s32 band_seed;
  s32 draw_seed;
  s32 seed;
  s32 x;
  s32 y;

  i = 0;
  band_seed = 0xE00;
  SPAD_REF(s16, 0x14u) = -0x1A40;
  SPAD_REF(s16, 0x16u) = -0x14C0;
  SPAD_REF(s16, 0x18u) = 0x160;
  SPAD_REF(s16, 0x1Cu) = 0x620;
  SPAD_REF(s16, 0x1Eu) = 0;
  for (; i < 4; i++) {
    draw_seed = band_seed;
    band_seed += 0x400;
    SPAD_REF(s16, 0x20u) = (s16)((D_80143E6C << 5) + (i << 10));
    func_801F46C8(workRecordCursor1F800044[9] << 2, 0x1F, draw_seed, 0x18);
  }
  if (workRecordCursor1F800044[9] >= 3u) {
    i = 0;
    vec = &D_1F800018;
    matrix = vec - 2;
    vec[0] = 0x3A0;
    SPAD_REF(s16, 0x1Cu) = 0x400;
    SPAD_REF(s16, 0x1Eu) = 0;
    for (k = 0; i < 4; i++, k += 2) {
      seed = i << 10;
      x = D_801F5764[k];
      matrix[0] = (s16)((((x * 3) << 16) + 0x4B8000) >> 9) - 0x4000;
      y = D_801F5765[k];
      matrix[6] = (s16)((D_80143E6C << 5) + seed);
      matrix[1] = (s16)((((y * 3) << 16) + 0x568000) >> 9) - 0x4000;
      func_801F46C8(0, 0x1E, seed, 0x10);
    }
  }
  func_801F4430();
  tick = workRecordCursor1F800044;
  tick[9]++;
  step = workRecordCursor1F800044;
  if (step[9] >= 8u) {
    step[2]++;
    func_8015DF18(0x200);
  }
}
