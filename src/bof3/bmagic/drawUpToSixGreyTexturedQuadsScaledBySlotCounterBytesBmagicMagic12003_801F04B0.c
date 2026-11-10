#include "bof3/bof3.h"
#include "gpu/prim.h"

typedef struct {
  s16 width;
  s16 unk_02;
  s16 unk_04;
  u16 shade;
} QuadParams;

extern QuadParams D_1F800000; /* @source 0x1F800000 @kind data */
extern SVECTOR D_1F800014[]; /* @source 0x1F800014 @kind data */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_801F0D0C[]; /* @source 0x801F0D0C @kind data */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* @source 0x801F04B0
 * @behavior Draw up to six semi-transparent grey textured quads under the
 *           installed transform. Slot i draws when 3*i is below work byte 9
 *           and counter byte 0x801F0D0C+i is below 16. Set the scratchpad
 *           half-extent to counter*4+50, and the four vector depths to
 *           counter*15. Scale phase samples at 0x200, 0x600, 0xE00 and 0xA00
 *           by the half-extent, shifted right twelve, for the vector x/y
 *           coordinates. Use texture page (0,1,0x340,0x100), clut (32,506),
 *           UV corners (8,64), (40,64), (8,96), (40,96), and grey shade
 *           97-counter*6. Project the vectors into a POLY_FT4 and append its
 *           40 bytes to output list 3. Takes no arguments and returns nothing.
 * @status partial
 * @match 93.51
 * @residual UV constant 8 is hoisted into s7, adding its save/restore and
 *           growing the frame from 0x50 to 0x58 and the body by eight bytes.
 */
void drawUpToSixGreyTexturedQuadsScaledBySlotCounterBytesBmagicMagic12003_801F04B0(void) {
  s32 i = 0;
  QuadParams *params = &D_1F800000;
  SVECTOR *vec = D_1F800014;
  u8 *counter = D_801F0D0C;
  s32 k = 0;
  s32 depth;
  s32 shade;
  POLY_FT4 *prim;
  long p;
  long flag;

  for (; i < 6; i++, k += 3) {
    if (k < g_battle_work[9] && *counter < 0x10) {
      prim = (POLY_FT4 *)g_PrimCursor;
      SetPolyFT4(prim);
      SetSemiTrans(prim, 1);
      params->width = *counter * 4 + 0x32;
      depth = *counter * 15;
      vec[0].vx = (func_801783C8(0x200) * params->width) >> 12;
      vec[0].vy = (func_801782FC(0x200) * params->width) >> 12;
      vec[0].vz = depth;
      vec[1].vx = (func_801783C8(0x600) * params->width) >> 12;
      vec[1].vy = (func_801782FC(0x600) * params->width) >> 12;
      vec[1].vz = depth;
      vec[2].vx = (func_801783C8(0xE00) * params->width) >> 12;
      vec[2].vy = (func_801782FC(0xE00) * params->width) >> 12;
      vec[2].vz = depth;
      vec[3].vx = (func_801783C8(0xA00) * params->width) >> 12;
      vec[3].vy = (func_801782FC(0xA00) * params->width) >> 12;
      vec[3].vz = depth;

      prim->tpage = GetTPage(0, 1, 0x340, 0x100);
      prim->clut = GetClut(0x20, 0x1FA);
      prim->u0 = 8;
      prim->v0 = 0x40;
      prim->u1 = 0x28;
      prim->v1 = 0x40;
      prim->u2 = 8;
      prim->v2 = 0x60;
      prim->u3 = 0x28;
      prim->v3 = 0x60;

      shade = 0x61 - *counter * 6;
      params->shade = shade;
      prim->r0 = shade;
      prim->g0 = params->shade;
      prim->b0 = params->shade;
      RotTransPers4(vec, vec + 1, vec + 2, vec + 3, (long *)&prim->x0,
                    (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                    &p, &flag);
      GpuAppendPrim(3, 0x28);
    }
    counter++;
  }
}
