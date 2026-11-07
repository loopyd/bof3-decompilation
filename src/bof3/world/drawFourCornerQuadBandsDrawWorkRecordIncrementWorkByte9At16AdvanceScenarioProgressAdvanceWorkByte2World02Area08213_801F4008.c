#include "bof3/bof3.h"

/* Shared per-frame counter at 0x80143E6C, bound by the shared map. This
 * handler reads it once per corner pass and shifts it left by five to seed
 * element 2 of the scratch rotation matrix. */
extern u32 D_80143E6C;

/* Shared scenario progress byte at 0x80146864, bound by the shared map. The
 * handler advances it once the work record's +0x09 step byte reaches 0x10. */
extern u8 g_ScenarioProgress;

/* Scratch work-record pointer cell published by the overlay. The original
 * re-reads it once per access group, re-materializing its address
 * (lui $vN,0x1F80 / lw $vN,68($vN)) for each read, so it is named through this
 * target's map instead of the fixed-address accessor. */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */

/* Overlay byte tables at 0x801F5764 and 0x801F5765: four signed off/on bytes
 * each, read with `lb` as the per-corner sign pair (-1,+1) of the scratch
 * translation vector. They sit inside the overlay's trailing byte data, which
 * still-unlifted sibling spans 0x801F3E08 and 0x801F4430 also reference, so
 * Splat resolves both addresses through this target's generated undefined
 * symbol map. */
extern s8 D_801F5764[]; /* @source 0x801F5764 @kind data */
extern s8 D_801F5765[]; /* @source 0x801F5765 @kind data */

/* Scratch translation-vector Z halfword at 0x1F800018, the component the
 * quad-band drawer's RotTrans call reads from the vector at 0x1F800014. The
 * original materialises its address as a value (lui plus addiu %lo) and
 * derives the 0x1F800014 base it indexes in the loop by subtracting 4, so the
 * cell is bound as a symbol instead of through the constant-address accessor,
 * whose materialisation would fold into lui plus ori. */
extern s16 D_1F800018; /* @source 0x1F800018 @kind unknown */

/* Quad-band drawer, the overlay's own routine at 0x801F46C8. It pushes the
 * current matrix, projects the scratch translation vector at 0x1F800014
 * through RotTrans and the scratch rotation matrix at 0x1F80001C through
 * RotMatrix, walks one row per index from its first argument up to its second
 * drawing two translucent POLY_G4 quads per row, and hands its third argument
 * to func_80179F98 together with the projected matrix. Splat binds the address
 * through this target's generated asm label. */
void func_801F46C8(s32 first_row, s32 row_limit, s32 matrix_seed, s32 row_scale);

/* Draw helper, the overlay's own routine at 0x801F4430. Its body rotates and
 * projects the work record fields +0x30/+0x32/+0x34, draws a run of four
 * textured quads, and, when the record's +0x0B byte is non-zero, hands that
 * byte plus 0x201 to the cue dispatcher func_8015DF18 and then clears the byte.
 * Splat binds the address through this target's generated asm label. */
void func_801F4430(void);

/* @source 0x801F4008
 * @behavior Overlay per-frame handler, entry index 5 of the eight-pointer
 *           byte-2 handler table at 0x801F576C, so the byte +0x02 advance below
 *           passes control to the following table entry at 0x801F4190. It
 *           seeds the scratch geometry first: 0x1F800018 takes 0x3A0 and
 *           0x1F80001C/0x1F80001E take 0x400 and 0, which is the translation
 *           Z and the leading rotation-matrix pair the quad-band drawer at
 *           0x801F46C8 feeds to RotTrans and RotMatrix. It then walks the four
 *           corners of the layout in a bottom-tested loop: the signed bytes of
 *           0x801F5764/0x801F5765 select each corner's translation X and Y as
 *           (corner * 3 << 16 plus a fixed bias) shifted right by nine and
 *           rebased by -0x4000, element 2 of the rotation matrix takes the
 *           shared frame counter 0x80143E6C shifted left by five plus the pass
 *           index shifted left by ten, and that same pass offset reaches the
 *           drawer as its third argument together with 0x1E as the row limit,
 *           0x10 as the per-row offset scale and the work record's +0x09 byte
 *           less eight, shifted left by two, as the first row. After the four
 *           passes it draws through the work-record helper at 0x801F4430,
 *           re-reads the work cell and advances the record's +0x09 byte, then
 *           re-reads the cell once more to test the incremented byte against
 *           0x10 as an unsigned bound: only once it has reached 16 does the
 *           handler advance the shared scenario progress byte at 0x80146864,
 *           clear +0x09 and, through a third cell read, advance that record's
 *           +0x02 dispatch byte by one. The function has a 0x20-byte frame
 *           holding the saved return address at 0x1C, takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFourCornerQuadBandsDrawWorkRecordIncrementWorkByte9At16AdvanceScenarioProgressAdvanceWorkByte2World02Area08213_801F4008(
    void) {
  s16* vec;
  s16* matrix;
  u8* tick;
  u8* step;
  u8* next;
  u8* progress;
  s32 i;
  s32 k;
  s32 seed;
  s32 x;
  s32 y;

  /* The pass index is zeroed ahead of the scratch base materialisation: the
   * original emits its `move $s1,$zero` before the 0x1F800018 address pair,
   * while the table byte offset is zeroed after that pair. */
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
    func_801F46C8((workRecordCursor1F800044[9] - 8) << 2, 0x1E, seed, 0x10);
  }
  func_801F4430();
  tick = workRecordCursor1F800044;
  tick[9]++;
  step = workRecordCursor1F800044;
  if (step[9] >= 0x10u) {
    progress = &g_ScenarioProgress;
    (*progress)++;
    step[9] = 0;
    next = workRecordCursor1F800044;
    next[2]++;
  }
}
