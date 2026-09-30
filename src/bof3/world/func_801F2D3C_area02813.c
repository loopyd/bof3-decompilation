#include "bof3/world/area02813_internal.h"

/* @source 0x801F2D3C
 * @behavior Re-seeds the current AREA028 work record and then keeps or drops
 *           the record's trailing dot depending on the whole 32-entry ring:
 *           record words 0x0C/0x0E receive the trail entry 0 halfwords
 *           (0x800E4A00/0x800E4A02) plus the record fields 0x04/0x06 shifted
 *           left by 7 and divided by the record scale; the ring is then walked
 *           from entry 0 and the walk abandons the dot as soon as the signed
 *           cross product of the step (entry i+1 over entry i) with the vector
 *           from entry i to the record centre turns negative. Only a fully
 *           walked ring stamps the shared primitive cursor with the 1x1 tile
 *           (TILE_1: the three colour bytes at 0x04/0x05/0x06 and the point at
 *           0x08/0x0A, a 0x0C-byte packet) at the record centre, its tint set
 *           from the record scale, and appends it to ordering-table head 3.
 * @status partial
 * @match 84.56
 * @residual live audit: 115/136 instructions, 544 original bytes versus 472 current; body, loop and tail byte-identical, the sole residual being the two maspsx sign-division trap sequences (break 7/break 6) that the canonical object profile omits and -Wa,--expand-div restores (that probe reproduces the original 544/544 bytes).
 */
void func_801F2D3C(void) {
  World00Area028Work* work;
  World00Area028Work* record;
  TILE_1* prim;
  u32 cur;
  u32 next;
  u8 nextIdx;
  SVECTOR delta;
  SVECTOR centre;
  s32 product;
  s8 tint;
  u8 i;

  work = workCursor;

  work->field_0c =
      WORLD00_AREA028_TRAIL[0].x + (s16)(((s32)work->field_04 << 7) / work->scale);
  work->field_0e =
      WORLD00_AREA028_TRAIL[0].y + (s16)(((s32)work->field_06 << 7) / work->scale);

  record = work;
  for (i = 0; i < 0x20; i++) {
    nextIdx = (u8)((i + 1) & 0x1F);
    next = (u32)nextIdx * 4u;
    cur = (u32)(i & 0xFF) * 4u;
    delta.vx =
        (s16)(WORLD00_AREA028_RING_X_AT(next) - WORLD00_AREA028_RING_X_AT(cur));
    delta.vy =
        (s16)(WORLD00_AREA028_RING_Y_AT(next) - WORLD00_AREA028_RING_Y_AT(cur));
    centre.vx = (s16)(record->field_0c - WORLD00_AREA028_RING_X_AT(cur));
    centre.vy = (s16)(record->field_0e - WORLD00_AREA028_RING_Y_AT(cur));
    product = (s32)delta.vx * centre.vy - (s32)delta.vy * centre.vx;
    if (product < 0) {
      return;
    }
  }

  prim = (TILE_1*)WORLD00_AREA028_PRIMITIVE_PTR;
  func_8017AA30(prim);
  SetSemiTrans(prim, 0);

  prim->x0 = workCursor->field_0c;
  prim->y0 = workCursor->field_0e;

  tint = -0x40 - (((s32)workCursor->scale - 0x80) * 3 << 6) / 512;
  prim->r0 = tint;
  prim->g0 = tint;
  prim->b0 = tint;

  func_8014E5A0(3, 0x0C);
}
