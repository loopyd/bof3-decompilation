#include "bof3/bof3.h"
#include "gpu/prim.h"

extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

s32 GetGraphType(void);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A97C(void *arg0);
s32 func_801782FC(s32 phase);
s32 func_801783C8(s32 phase);

/* @source 0x801EF2F4
 * @behavior Draw sixteen translucent Gouraud triangles around the unsigned
 *           work halfwords 0x2E and 0x30. The radius is the incoming word plus
 *           bit zero of D_80143E6C; successive rim vertices use phases i<<8
 *           and (i+1)<<8, with fixed-point samples shifted right twelve.
 *           Each pass emits a draw-mode packet followed by a triangle through
 *           func_80155A08 at work words 0x34 and 0x38, kind zero, sizes 0xC
 *           and 0x1C. The centre is grey 0x60 and both rim colours are
 *           (0x20,0x20,0x40). The draw-mode page is 0xA5 for graph type one
 *           or two, otherwise 0x35, querying separately at each test.
 *           No direct caller or address word occurs in this payload; the
 *           incoming radius role follows its signed products in this body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSixteenTriangleFanWithParityJitterBmagicMagic07503_801EF2F4(s32 radius) {
  POLY_G3 *prim;
  s32 i;
  s32 phase;
  s32 nextPhase;
  s32 graph;

  radius += D_80143E6C & 1;
  i = 0;
  do {
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      graph = 0xA5;
    } else {
      graph = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 0, 0xC);
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    prim->x0 = *(u16 *)(g_battle_work + 0x2E);
    prim->y0 = *(u16 *)(g_battle_work + 0x30);
    phase = i << 8;
    prim->x1 = *(u16 *)(g_battle_work + 0x2E) +
               ((func_801782FC(phase) * radius) >> 12);
    prim->y1 = *(u16 *)(g_battle_work + 0x30) +
               ((func_801783C8(phase) * radius) >> 12);
    nextPhase = (i + 1) << 8;
    prim->x2 = *(u16 *)(g_battle_work + 0x2E) +
               ((func_801782FC(nextPhase) * radius) >> 12);
    i++;
    prim->y2 = *(u16 *)(g_battle_work + 0x30) +
               ((func_801783C8(nextPhase) * radius) >> 12);
    prim->r0 = 0x60;
    prim->g0 = 0x60;
    prim->b0 = 0x60;
    prim->r1 = 0x20;
    prim->g1 = 0x20;
    prim->b1 = 0x40;
    prim->r2 = 0x20;
    prim->g2 = 0x20;
    prim->b2 = 0x40;
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 0, 0x1C);
  } while (i < 0x10);
}
