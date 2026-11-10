#include "bof3/bof3.h"
#include "gpu/prim.h"

extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

s32 GetGraphType(void);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_801782FC(s32 phase);
s32 func_801783C8(s32 phase);

/* @source 0x801EF500
 * @behavior Draw sixteen translucent Gouraud quads between two incoming radii
 *           around unsigned work halfwords 0x2E and 0x30. Add bit zero of
 *           D_80143E6C to both radii and sample successive phases i<<8 and
 *           (i+1)<<8, shifting signed fixed-point products right twelve.
 *           Vertices zero and one use the first radius and RGB (0x20,0x20,0x40);
 *           vertices two and three use the second radius and RGB (1,1,1).
 *           Emit draw-mode and quad packets through func_80155A08 at work
 *           words 0x34 and 0x38, kind zero, sizes 0xC and 0x24. Select page
 *           0xA5 for graph type one or two, otherwise 0x35, querying each test.
 *           No direct caller or address word occurs in this payload; the
 *           argument roles follow their signed products in this body.
 * @status partial
 * @match 89.88
 * @residual Saved-register allocation differs for the loop index and both radii.
 */
void drawSixteenGouraudRingQuadsWithParityJitterBmagicMagic07503_801EF500(s32 radius0, s32 radius1) {
  POLY_G4 *prim;
  s32 i;
  s32 phase;
  s32 nextPhase;
  s32 graph;

  radius0 += D_80143E6C & 1;
  radius1 += D_80143E6C & 1;
  i = 0;
  do {
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      graph = 0xA5;
    } else {
      graph = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 0, 0xC);
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    phase = i << 8;
    prim->x0 = *(u16 *)(g_battle_work + 0x2E) +
               ((func_801782FC(phase) * radius0) >> 12);
    prim->y0 = *(u16 *)(g_battle_work + 0x30) +
               ((func_801783C8(phase) * radius0) >> 12);
    i++;
    nextPhase = i << 8;
    prim->x1 = *(u16 *)(g_battle_work + 0x2E) +
               ((func_801782FC(nextPhase) * radius0) >> 12);
    prim->y1 = *(u16 *)(g_battle_work + 0x30) +
               ((func_801783C8(nextPhase) * radius0) >> 12);
    prim->x2 = *(u16 *)(g_battle_work + 0x2E) +
               ((func_801782FC(phase) * radius1) >> 12);
    prim->y2 = *(u16 *)(g_battle_work + 0x30) +
               ((func_801783C8(phase) * radius1) >> 12);
    prim->x3 = *(u16 *)(g_battle_work + 0x2E) +
               ((func_801782FC(nextPhase) * radius1) >> 12);
    prim->y3 = *(u16 *)(g_battle_work + 0x30) +
               ((func_801783C8(nextPhase) * radius1) >> 12);
    prim->r0 = 0x20;
    prim->g0 = 0x20;
    prim->b0 = 0x40;
    prim->r1 = 0x20;
    prim->g1 = 0x20;
    prim->b1 = 0x40;
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;
    prim->r3 = 1;
    prim->g3 = 1;
    prim->b3 = 1;
    func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 0, 0x24);
  } while (i < 0x10);
}
