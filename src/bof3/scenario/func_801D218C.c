#include "bof3/scenario/sce10eff_internal.h"
#include "gpu/prim.h"
#include <stdlib.h>

/* @source 0x801D218C
 * @behavior Seeds the scratchpad cells at 0x1F800000/0x1F800004/0x1F800008/
 * 0x1F80000C (block size 0x40, angular phase from work byte 0x0B plus two,
 * primitive count from work byte 0x0A, random colour seed), samples the first
 * radial vertex at 0x1F800014 through the local rotation helper func_801782FC
 * scaled by the block size and shifted right 12, then repeatedly emits one
 * semi-transparent two-vertex Gouraud primitive: seeds the draw mode from
 * GetGraphType (0x2A5 for graphics types 1 and 2, 0xB5 otherwise), projects the
 * previous radial vertex through RotTransPers into the primitive vertex at
 * 0x08, re-randomizes the block size to 0x80 plus or minus 0..0x3F, advances
 * the phase by two from work byte 0x0B, samples the next radial vertex (depth
 * -(index << 6)) and projects it into the primitive vertex at 0x10, writes the
 * random colour into primitive bytes 0x04/0x05 and 0x0C/0x0D with 0x20 in
 * bytes 0x06/0x0E, and queues each primitive with func_8014E5A0(2, 0x14) until
 * the index reaches the count.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D218C(void) {
  s32*          base;
  s32*          stage;
  u8*           prim;
  s32           i;
  s32           graph_value;
  s32           count_or_ready;
  s32           rotation_or_product;
  s32           sample_or_scale;
  s32           phase;
  s32           projected;
  s32           depth;

  base = (s32*)&D_1F800000;
  *base = 0x40;
  D_1F800004 = (s32)((s32)(((((u8*)D_1F800044)[0x0b]) + 2) & 0x0fu) << 8);
  D_1F80000C = ((u8*)D_1F800044)[0x0a];
  *(s32*)&D_1F800008 = rand() & 0x3f;
  i = 1;
  D_1F800014.vx = 0;
  rotation_or_product = func_801782FC(D_1F800004) * *base;
  D_1F800014.vz = 0;
  stage = (s32*)(base + 1);
  count_or_ready = stage[2];
  count_or_ready = i < count_or_ready;
  sample_or_scale = rotation_or_product >> 12;
  D_1F800014.vy = (s16)sample_or_scale;
  while (count_or_ready) {
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      graph_value = 0x2a5;
    } else {
      graph_value = 0xb5;
    }

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, graph_value, 0);
    func_8014E5A0(2, 0x0c);

    prim = g_PrimCursor;
    func_8017AA94(prim);
    SetSemiTrans(prim, 1);

    RotTransPers(&D_1F800014, (long*)(prim + 8), (long*)&projected,
                 (long*)&depth);

    if (rand() & 1) {
      D_1F800000 = (rand() & 0x3f) + 0x80;
    } else {
      D_1F800000 = 0x80 - (rand() & 0x3f);
    }

    phase = (s32)((s32)(((((u8*)D_1F800044)[0x0b]) + i + 2) & 0x0fu) << 8);
    D_1F800014.vx = 0;
    stage[0] = phase;
    rotation_or_product = func_801782FC(phase);
    sample_or_scale = stage[-1];
    D_1F800014.vy = (s16)((rotation_or_product * sample_or_scale) >> 12);
    D_1F800014.vz = (s16)(-(i << 6));

    RotTransPers(&D_1F800014, (long*)(prim + 0x10), (long*)&projected,
                 (long*)&depth);

    prim[4] = stage[1];
    prim[5] = stage[1];
    prim[6] = 0x20;

    stage[1] = (rand() + 0x40) & 0x3f;
    prim[0xc] = stage[1];
    prim[0xd] = stage[1];
    prim[0xe] = 0x20;

    func_8014E5A0(2, 0x14);

    i++;
    count_or_ready = stage[2];
    count_or_ready = i < count_or_ready;
  }
}
