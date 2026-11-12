#include "bof3/world/area03004_internal.h"

extern void func_8017AA80(void* arg0); /* @source 0x8017AA80 @kind function */

/* @behavior While the area gate byte D_80149332 is clear it drives the shared
 * view pair: when the shared pad word D_80145AA4 holds bit 0x10 or the shared
 * view y word D_8014930C already equals 0x150000, and the shared mode byte is
 * clear, it arms the view scale halfword D_8014932E with 0x40, snaps D_8014930C
 * to 0x3C0000 and stores 1 into byte 3 of the scratch work record published at
 * the scratchpad cursor 0x1F800044; otherwise it arms D_8014932E with 0x20 and
 * steps D_8014930C down by 0x8000. Then, only on the frames whose shared frame
 * counter D_80143E6C holds bit 2, it emits five 0x10-byte grey (0x80) projected
 * line primitives at ordering-table slot 2: each step takes g_PrimCursor,
 * hands it to func_8017AA80, projects a local vertex whose x is -0x4000 and
 * then -0x3100 with z 0 and whose y is ((0x3E0000 - phase) >> 9) - 0x4000 into
 * the primitive vertex pairs at offsets 0x08/0x0A and 0x0C/0x0E, advances the
 * phase by 0xA0000 per step, and appends the primitive through GpuAppendPrim.
 * Called without arguments by the area030 dispatch chain; returns nothing.
 * @source 0x801DD9A0
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepSharedViewYAndDrawFiveGreyLinesWorld00Area03004_801DD9A0(void) {
  u8* primitive;
  u8 tint;
  u16* sxyPtr;
  s32* depthPtr;
  s32* flagPtr;
  u32* viewY;
  SVECTOR shape;
  u16 xy[2];
  s32 projected;
  s32 flags;
  s32 phase;
  s32 i;

  if (D_80149332 == 0) {
    if (((D_80145AA4 & 0x10) != 0 || D_8014930C == 0x150000) && modeByte == 0) {
      D_8014932E = 0x40;
      D_8014930C = 0x3C0000;
      D_1F800044[3] = 1;
    } else {
      viewY = &D_8014930C;
      D_8014932E = 0x20;
      *viewY -= 0x8000;
    }
  }

  if ((D_80143E6C & 4) == 0) {
    return;
  }

  i = 1;
  sxyPtr = xy;
  depthPtr = &projected;
  flagPtr = &flags;
  tint = 0x80;
  phase = 0xA0000;
  do {
    primitive = g_PrimCursor;
    i++;
    func_8017AA80(primitive);
    shape.vx = -0x4000;
    shape.vy = ((0x3E0000 - phase) >> 9) - 0x4000;
    shape.vz = 0;
    RotTransPers(&shape, (long*)sxyPtr, (long*)depthPtr, (long*)flagPtr);
    *(u16*)(primitive + 8) = xy[0];
    *(u16*)(primitive + 0x0A) = xy[1];
    shape.vx = -0x3100;
    shape.vy = ((0x3E0000 - phase) >> 9) - 0x4000;
    shape.vz = 0;
    RotTransPers(&shape, (long*)sxyPtr, (long*)depthPtr, (long*)flagPtr);
    *(u16*)(primitive + 0x0C) = xy[0];
    *(u16*)(primitive + 0x0E) = xy[1];
    primitive[4] = tint;
    primitive[5] = tint;
    primitive[6] = tint;
    GpuAppendPrim(2, 0x10);
    phase += 0xA0000;
  } while (i < 6);
}
