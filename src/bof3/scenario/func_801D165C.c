#include "bof3/scenario/sce10eff_internal.h"
#include "gpu/prim.h"
#include <stdlib.h>

/* @source 0x801D165C
 * @behavior Emits the translucent radial quad bands of the scenario overlay:
 * seeds the scratchpad block size at 0x1F800000 with 0x10, the angular phase
 * cell at 0x1F800004 with the low nibble of work byte 0x0B shifted left eight,
 * the colour cell at 0x1F800008 with seven times work byte 0x0A and the first
 * radial sample at 0x1F800014, then walks seventeen bands. Each band advances
 * the phase by the band index, re-seeds the block size from the caller's
 * halfwidth plus or minus a random fraction of the caller's jitter, copies the
 * previous radial sample into the two far vertices of the eight-halfword band
 * shifted down by the caller's thickness, samples the new radial pair (depth
 * -(index << 6)) through func_801782FC scaled by the block size, and builds a
 * four-vertex Gouraud quad with SetPolyG4/SetSemiTrans whose colours are the
 * thick-rim shade, the random colour cell and white. After projecting it with
 * RotTransPers4, while its second vertex keeps a positive screen y it emits
 * three more 0x24-byte quads through func_8014E5A0(2, 0x24) from the same
 * scratchpad rotation frame, shifting the band halfwords by the caller's
 * thickness between them (down, up by twice, up) and finishing with a
 * two-thickness shift of the first sampled vertex; on the first band the two
 * previous-sample vertices of every quad are painted white.
 * @status partial
 * @match 85.46
 * @residual the frame size now matches the original (0x70) and the first
 * difference is +0x0018: the original homes the halfwidth parameter with
 * `sh a1,48(sp)` and then reloads the hoisted thickness promotion from a stack
 * slot inside both arms, while this build loads the scratch pointer into $a0
 * (the original uses $a1) and leaves the halfwidth un-homed, so the prologue
 * inset order and the arm addends still differ. The rest is the allocator
 * class: the work pointer lands in $a0 rather than $a1, so the angular phase
 * reaches the func_801782FC argument through an extra move, and one band-setup
 * store group is ordered differently. Clean-C levers measured and rejected: the
 * raw 0x1F8000xx halfword macros (they hoist a base register instead of folding
 * per access -> 52.57%), volatile views of the same cells (53.02%), a promoted
 * halfwidth local for the parameter arms (no change), scratchpad-local copies
 * of the sampled pair (52.57% before the folded addressing was restored, 81.74%
 * after), moving the band-setup store group after the rotation call (78.84%), a
 * block-local jitter re-materialisation at the loop head or per arm (371/450,
 * frame 0x74), a hoisted non-volatile pointer to the jitter home (359/448), a
 * `(s16)` cast on the jitter operand (367/449, unchanged) and volatile home
 * reads of both parameters (341/450). The retained lever is the volatile home
 * read of the jitter alone: it stops the loop optimiser from keeping the
 * parameter's sign-extension live in a register across the loop, restoring the
 * original's per-arm `lhu` of the parameter home (367/449 -> 382/447).
 */
void func_801D165C(s32 arg0, s16 arg1, s16 arg2) {
  s32*     size;
  s32*     stage;
  s32*     color;
  u16*     band;
  SVECTOR* vertices;
  u8*      scratch;
  POLY_G4* prim;
  s32      graph_value;
  s32      phase;
  s32      shade;
  u16      prev_y;
  u16      prev_z;
  s32      i;
  long     depth;
  long     flag;

  scratch = (u8*)D_1F800044;
  size = (s32*)&D_1F800000;
  *size = 0x10;
  phase = (s32)(((scratch[0x0b] & 0x0fu) << 8));
  D_1F800004 = phase;
  D_1F800008 = scratch[0x0a] * 7;
  shade = scratch[0x0a] * 13;
  D_1F800014.vz = 0;
  D_1F800014.vy = (func_801782FC(phase) * *size) >> 12;

  if (GetGraphType() == 1) {
    graph_value = 0xa5;
  } else if (GetGraphType() == 2) {
    graph_value = 0xa5;
  } else {
    graph_value = 0x35;
  }

  band = (u16*)&D_1F800014 + 8;
  vertices = (SVECTOR*)(band - 8);
  stage = (s32*)&D_1F800004;
  i = 1;
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, graph_value, 0);
  func_8014E5A0(2, 12);
  color = stage + 1;

  for (i = 1; i < 0x12; i++) {
    scratch = (u8*)D_1F800044;
    phase = (s32)(((scratch[0x0b] + i) & 0x0fu) << 8);
    stage[0] = phase;

    if (rand() & 1) {
      stage[-1] = arg1 + (rand() & *(volatile s16*)&arg2);
    } else {
      stage[-1] = arg1 - (rand() & *(volatile s16*)&arg2);
    }

    prev_y = band[-7];
    prev_z = band[-6];
    band[0] = 0;
    band[4] = 0;
    band[-8] = 0;
    band[1] = prev_y;
    band[2] = prev_z;
    band[5] = prev_y - arg0;
    band[6] = prev_z;
    band[-4] = 0;
    band[-6] = (u16)(-(i << 6));
    band[-2] = (u16)(-(i << 6));
    band[-7] = (u16)((func_801782FC(stage[0]) * stage[-1]) >> 12);
    band[-3] = band[-7] - arg0;

    prim = (POLY_G4*)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    prim->r0 = stage[1];
    prim->g0 = stage[1];
    prim->b0 = stage[1];
    prim->r1 = shade;
    prim->g1 = stage[1];
    prim->b1 = 1;
    if (i == 1) {
      prim->r2 = i;
      prim->g2 = i;
      prim->b2 = i;
      prim->r3 = i;
      prim->g3 = i;
      prim->b3 = i;
    } else {
      prim->r2 = stage[1];
      prim->g2 = stage[1];
      prim->b2 = stage[1];
      prim->r3 = shade;
      prim->g3 = stage[1];
      prim->b3 = 1;
    }

    RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                  (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                  (long*)&prim->x3, &depth, &flag);

    if (prim->y2 > 0) {
      func_8014E5A0(2, 0x24);

      prim = (POLY_G4*)g_PrimCursor;
      D_1F800016 =
          D_1F800016 - arg0;
      D_1F80001E =
          D_1F80001E - arg0;
      D_1F800026 =
          D_1F800026 - arg0;
      D_1F80002E =
          D_1F80002E - arg0;
      SetPolyG4(prim);
      SetSemiTrans(prim, 1);
      prim->r0 = shade;
      prim->g0 = color[0];
      prim->b0 = 1;
      prim->r1 = 1;
      prim->g1 = 1;
      prim->b1 = 1;
      if (i == 1) {
        prim->r2 = 1;
        prim->g2 = 1;
        prim->b2 = 1;
        prim->r3 = 1;
        prim->g3 = 1;
        prim->b3 = 1;
      } else {
        prim->r2 = shade;
        prim->g2 = color[0];
        prim->b2 = 1;
        prim->r3 = 1;
        prim->g3 = 1;
        prim->b3 = 1;
      }

      RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                    (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                    (long*)&prim->x3, &depth, &flag);
      func_8014E5A0(2, 0x24);

      prim = (POLY_G4*)g_PrimCursor;
      D_1F800016 =
          D_1F800016 + (s16)arg0 * 2;
      D_1F80001E =
          D_1F80001E + (s16)arg0 * 2;
      D_1F800026 =
          D_1F800026 + (s16)arg0 * 2;
      D_1F80002E =
          D_1F80002E + (s16)arg0 * 2;
      SetPolyG4(prim);
      SetSemiTrans(prim, 1);
      prim->r0 = shade;
      prim->g0 = color[0];
      prim->b0 = 1;
      prim->r1 = color[0];
      prim->g1 = color[0];
      prim->b1 = color[0];
      if (i == 1) {
        prim->r2 = 1;
        prim->g2 = 1;
        prim->b2 = 1;
        prim->r3 = 1;
        prim->g3 = 1;
        prim->b3 = 1;
      } else {
        prim->r2 = shade;
        prim->g2 = color[0];
        prim->b2 = 1;
        prim->r3 = color[0];
        prim->g3 = color[0];
        prim->b3 = color[0];
      }

      RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                    (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                    (long*)&prim->x3, &depth, &flag);
      func_8014E5A0(2, 0x24);

      prim = (POLY_G4*)g_PrimCursor;
      D_1F800016 =
          D_1F800016 + arg0;
      D_1F80001E =
          D_1F80001E + arg0;
      D_1F800026 =
          D_1F800026 + arg0;
      D_1F80002E =
          D_1F80002E + arg0;
      SetPolyG4(prim);
      SetSemiTrans(prim, 1);
      prim->r0 = 1;
      prim->g0 = 1;
      prim->b0 = 1;
      prim->r1 = shade;
      prim->g1 = color[0];
      prim->b1 = 1;
      if (i == 1) {
        prim->r2 = 1;
        prim->g2 = 1;
        prim->b2 = 1;
        prim->r3 = 1;
        prim->g3 = 1;
        prim->b3 = 1;
      } else {
        prim->r2 = 1;
        prim->g2 = 1;
        prim->b2 = 1;
        prim->r3 = shade;
        prim->g3 = color[0];
        prim->b3 = 1;
      }

      RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                    (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                    (long*)&prim->x3, &depth, &flag);
      func_8014E5A0(2, 0x24);

      D_1F800016 =
          D_1F800016 - (s16)arg0 * 2;
    }
  }
}
