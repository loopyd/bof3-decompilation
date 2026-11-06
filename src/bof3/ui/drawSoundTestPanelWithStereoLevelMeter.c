#include "bof3/context.h"
#include "gpu/prim.h"

/* @source 0x8014E5A0 */
extern void appendRenderPrim(u32 ot_index, u32 primitive_size);

/* @source 0x801D1714 */
extern void drawGlyphStringQuadsCommu03_06(s32 x, s32 y, u32 palette,
                                           const u8* text);

/* @source 0x801682E0 */
extern s32 SpuGetKeyStatus(u32 key);

/* @source 0x801AE3F0 */
extern void func_801AE3F0(int arg0, int arg1, u32 arg2, u32 arg3, u32 arg4,
                          u32 arg5);

/* @source 0x801AED54 */
extern void func_801AED54(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* @source 0x8014FC90 */
extern void func_8014FC90(s16 arg0, s16 arg1, s32 arg2, u32 arg3,
                          const u8* arg4);

/* @source 0x80169174 */
extern void func_80169174(u32 voice, s16* left, s16* right);

/* @source 0x800104E2 @kind table */
extern u16 D_800104E2[];
/* @source 0x80144951 @kind unknown */
extern u8 D_80144951;
/* @source 0x80144952 @kind unknown */
extern s8 D_80144952;
/* @source 0x80145029 @kind unknown */
extern u8 D_80145029;
/* @source 0x801D18F4 @kind unknown */
extern const u8 D_801D18F4[];
/* @source 0x801D18F8 @kind unknown */
extern const u8 D_801D18F8[];
/* @source 0x801D18FC @kind unknown */
extern const u8 D_801D18FC[];
/* @source 0x801D1904 @kind unknown */
extern const u8* D_801D1904;
/* @source 0x801D1908 @kind unknown */
extern const u8* D_801D1908;
/* @source 0x801D190C @kind unknown */
extern const u8* D_801D190C;
/* @source 0x801D1918 @kind table */
extern u8 D_801D1918[];
/* @source 0x801D1940 @kind unknown */
extern s16 D_801D1940;

/* @source 0x801D1250
 * @behavior draws the sound-test panel at (x, y): the shaded backdrop through
 *           func_801AE3F0 and three inset rectangles through func_801AED54, the
 *           pre-rendered title texture selected by the D_801D1940 entry index
 *           through func_8014F800 with justification 2 when the D_80145029 byte
 *           matches that index's D_801D1918 entry and 0 otherwise, a second
 *           func_801AE3F0/func_801AED54 rectangle pair for the option row, the
 *           "STEREO" string of D_801D1908 when D_80144951 is set and the
 *           "TITLE" string of D_801D1904 when it is clear, the "MONO" string of
 *           D_801D190C, the ">>>" marker D_801D18F4 at two option columns with
 *           the D_801D18F8 and D_801D18FC icon records between them through
 *           func_8014FC90, then a stereo level meter. The meter asks
 *           SpuGetKeyStatus for each of the twenty-four SPU voices and, for
 *           every active voice, reads that voice's two signed level halfwords
 *           through func_80169174, divides each by 300 and accumulates them
 *           into two totals, which are clamped to 0..0x7A. Two 0x28-byte
 *           textured bars are then built at x+0x49 and y+0x3F/y+0x43 with the
 *           channel total as their length, texture window 0x98..0x9C by
 *           0xD8..0xE0, CLUT 0x780B and texture page 0xF.
 * @status partial
 * @match 89.84
 * @residual allocation, clamp control flow and bar-store scheduling:
 *           size and instruction count agree at 1220 bytes and 305 instructions,
 *           but the two func_8014F800 arms read saved px/py (s4/s5) where the
 *           original reads the formals (s2/s3). The accumulator clamps use
 *           in-place results, inverted second tests and empty delay slots.
 *           The left-edge local shares px's register rather than taking its
 *           own register, and the bar primitive's stores are scheduled
 *           differently. The table address also reverses the addu operands.
 *           The saved x/y copies are the original's own: 0x801D12FC copies x
 *           into a second register and 0x801D1324 copies y in the bne delay
 *           slot, and the tail reads that pair while the two func_8014F800
 *           arms read the formals.
 */
void drawSoundTestPanelWithStereoLevelMeter(s32 x, s32 y) {
  POLY_FT4* primitive;
  s16 sx;
  s16 sy;
  s32 totals[2];
  s16 levels[2];
  s32 limit;
  s32 limit2;
  s32 left;
  s32 px;
  s32 py;
  s32 i;

  sx = (s16)x;
  sy = (s16)y;
  func_801AE3F0(sx, sy, 0xDC, 0x6E, 0xF0, D_80144952);
  func_801AED54(sx + 4, sy + 4, 0xD2, 0x66, 0);
  func_801AED54(sx + 8, sy + 8, 0xCA, 0x5E, 0);
  func_801AED54(sx + 0x10, sy + 0x12, 0xBC, 0x22, 1);

  px = x;
  py = y;

  if (D_80145029 == D_801D1918[D_801D1940]) {
    func_8014F800((s16)(x + 0x14), (s16)(y + 0x17), 2, 0xFFu,
                  0x80010000u + (u32)D_800104E2[D_801D1940]);
  } else {
    func_8014F800((s16)(x + 0x14), (s16)(y + 0x17), 0, 0xFFu,
                  0x80010000u + (u32)D_800104E2[D_801D1940]);
  }

  func_801AE3F0((s16)px + 0x48, (s16)py + 0x3C, 0x7C, 0x0E, 1, D_80144952);
  func_801AED54((s16)px + 0x48, (s16)py + 0x3C, 0x7C, 0x0E, 1);

  if (D_80144951 == 0) {
    drawGlyphStringQuadsCommu03_06((s16)(px + 0x10), (s16)(py + 0x40), 0,
                                   D_801D1904);
  } else {
    drawGlyphStringQuadsCommu03_06((s16)(px + 0x10), (s16)(py + 0x40), 0,
                                   D_801D1908);
  }

  sy = (s16)(py + 0x54);
  drawGlyphStringQuadsCommu03_06((s16)(px + 0x20), sy, 0, D_801D190C);
  func_8014FC90((s16)(px + 0x44), sy, 0, 3, D_801D18F4);
  func_8014FC90((s16)(px + 0x5C), sy, 3, 1, D_801D18F8);
  func_8014FC90((s16)(px + 0x94), sy, 0, 3, D_801D18F4);
  func_8014FC90((s16)(px + 0xAC), sy, 3, 1, D_801D18FC);

  totals[0] = 0;
  totals[1] = 0;
  for (i = 0; i < 0x18; i++) {
    if (SpuGetKeyStatus(1 << i) != 0) {
      func_80169174((u32)i, &levels[0], &levels[1]);
      totals[0] += (s16)(levels[0] / 300);
      totals[1] += (s16)(levels[1] / 300);
    }
  }

  limit = totals[0];
  if (limit >= 0) {
    if (limit > 0x7A) {
      limit = 0x7A;
    }
  } else {
    limit = 0;
  }
  totals[0] = limit;

  limit2 = totals[1];
  if (limit2 >= 0) {
    if (limit2 > 0x7A) {
      limit2 = 0x7A;
    }
  } else {
    limit2 = 0;
  }
  totals[1] = limit2;

  for (i = 0; i < 2; i++) {
    primitive = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(primitive);
    left = px + 0x49;
    primitive->x0 = left;
    primitive->y0 = py + 0x3F + i * 4;
    primitive->y1 = py + 0x3F + i * 4;
    primitive->u0 = 0x98;
    primitive->u1 = 0x9C;
    primitive->v0 = 0xD8;
    primitive->v1 = 0xD8;
    primitive->clut = 0x780B;
    primitive->x2 = primitive->x0;
    primitive->x1 = left + totals[i];
    primitive->y2 = primitive->y0 + 4;
    primitive->y3 = primitive->y2;
    primitive->r0 = 0x80;
    primitive->g0 = 0x80;
    primitive->b0 = 0x80;
    primitive->u3 = primitive->u1;
    primitive->u2 = primitive->u0;
    primitive->v2 = primitive->v0 + 8;
    primitive->v3 = primitive->v2;
    primitive->x3 = primitive->x1;
    if (GetGraphType() != 1) {
      GetGraphType();
    }
    primitive->tpage = 0xF;
    appendRenderPrim(1, 0x28);
  }
}
