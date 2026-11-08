#include "bof3/bof3.h"
#include "gpu/prim.h"

void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801F3AD8
 * @behavior Draws one screen-space record as a ring of eight Gouraud quads.
 *           It first sets the 0x380/0x100 texture page draw mode at the current
 *           primitive cursor with the dither enable raised, and links that
 *           0xC-byte packet through the shared grid-cell linker func_80155A08
 *           with the target-local word pair at 0x801F53A0/0x801F53A4 as its two
 *           coordinates. Then, for eight steps, it emits two 0x24-byte POLY_G4
 *           packets from the same cursor, one per 0x100 of turn so that the
 *           sixteen vertices close the full 0x1000 turn: every vertex puts its
 *           point pair at x = the record +0x0 halfword plus the +0x4 reach
 *           halfword scaled by the fixed-point x helper func_801783C8 of the
 *           phase shifted right 12, from y = 0 down to the record +0x2 halfword
 *           plus that reach scaled by the y helper func_801782FC shifted right
 *           13, so each quad is a vertical strip whose two left points share
 *           one phase and whose two right points share the next. All six
 *           channel bytes of the packet take the record +0x8 halfword clamped
 *           into 0x00..0xFF, and that shade walks four units per step, upwards
 *           for the first two steps and downwards afterwards. Each packet is
 *           linked through the same linker as grid kind 4 and both vertices of
 *           a step share one packet. Afterwards it restores the same texture
 *           page draw mode with the dither enable cleared through its own
 *           0xC-byte linked packet.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRadialGouraudQuadBurstAroundPointWorld01Area06714_801F3AD8(u8* point) {
  POLY_G4* quad;
  u16 phase;
  s16 shade;
  u8 color;
  u8 i;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x380, 0x100), 0);
  phase = 0;
  i = 0;
  func_80155A08(PSX_REF(s32, 0x801F53A0u), PSX_REF(s32, 0x801F53A4u), 4, 0xC);

  shade = FIELD_REF(u16, point, 0x8u);
  do {
    quad = (POLY_G4*)g_PrimCursor;
    SetPolyG4(quad);
    SetSemiTrans(quad, 1);
    quad->x0 = quad->x1 =
        FIELD_REF(u16, point, 0x0u) +
        ((FIELD_REF(s16, point, 0x4u) * func_801783C8(phase)) >> 12);
    quad->y0 = 0;
    quad->y1 =
        FIELD_REF(u16, point, 0x2u) +
        ((FIELD_REF(s16, point, 0x4u) * func_801782FC(phase)) >> 13);
    color = shade;
    if (shade >= 0x100) {
      color = 0xFF;
    }
    if (shade < 0) {
      color = 0;
    }
    quad->r0 = color;
    quad->g0 = color;
    quad->b0 = color;
    quad->r1 = color;
    quad->g1 = color;
    quad->b1 = color;
    if (i < 2) {
      shade += 4;
    } else {
      shade -= 4;
    }

    phase += 0x100;
    quad->x2 = quad->x3 =
        FIELD_REF(u16, point, 0x0u) +
        ((FIELD_REF(s16, point, 0x4u) * func_801783C8(phase)) >> 12);
    quad->y2 = 0;
    quad->y3 =
        FIELD_REF(u16, point, 0x2u) +
        ((FIELD_REF(s16, point, 0x4u) * func_801782FC(phase)) >> 13);
    color = shade;
    if (shade >= 0x100) {
      color = 0xFF;
    }
    if (shade < 0) {
      color = 0;
    }
    quad->r2 = color;
    quad->g2 = color;
    quad->b2 = color;
    quad->r3 = color;
    quad->g3 = color;
    quad->b3 = color;
    func_80155A08(PSX_REF(s32, 0x801F53A0u), PSX_REF(s32, 0x801F53A4u), 4, 0x24);
    i++;
  } while (i < 8);

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x380, 0x100), 0);
  func_80155A08(PSX_REF(s32, 0x801F53A0u), PSX_REF(s32, 0x801F53A4u), 4, 0xC);
}
