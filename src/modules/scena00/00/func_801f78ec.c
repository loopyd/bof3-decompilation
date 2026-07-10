#include "internal.h"

/* does: transforms two sets of 3d world-space coordinates into screen-space
 * sprite positions, using a shared global object. subtracts 0x4000 from each
 * coordinate (centre offset), halves and negates the rotation angles, then
 * calls the projection helper for each set. assigns colour channel values to
 * the object and schedules a DMA transfer.
 * @source: 0x801f78ec FUN_801f78ec
 */
void func_801f78ec(s32 x0, s32 y0, s16 angle0, s32 x1, s32 y1, s16 angle1, u8 r,
                   u8 g, u8 b) {
  volatile void* global_obj;
  volatile s16*  coord_stack;
  s16            sx;
  s16            sy;
  s16            srot;

  global_obj = (volatile void*)REG32(0x8014598cu);

  ((void (*)(volatile void*))0x8017aa80u)(global_obj);

  coord_stack = (volatile s16*)((volatile u8*)global_obj + 8u);

  sx = (s16)((x1 >> 9) - 0x4000);
  sy = (s16)((y1 >> 9) - 0x4000);
  srot = (s16)(-(angle1) / 2);

  ((void (*)(volatile s16*, volatile s16*, volatile void*,
             volatile void*))0x801794c8u)(
      coord_stack, coord_stack,
      (volatile void*)((volatile u8*)global_obj + 0x18u),
      (volatile void*)((volatile u8*)global_obj + 0x1cu));

  sx = (s16)((x0 >> 9) - 0x4000);
  sy = (s16)((y0 >> 9) - 0x4000);
  srot = (s16)(-(angle0) / 2);

  ((void (*)(volatile s16*, volatile s16*, volatile void*,
             volatile void*))0x801794c8u)(
      coord_stack, (volatile s16*)((volatile u8*)global_obj + 0xcu),
      (volatile void*)((volatile u8*)global_obj + 0x18u),
      (volatile void*)((volatile u8*)global_obj + 0x1cu));

  REG8((u32)((volatile u8*)global_obj + 4u)) = r;
  REG8((u32)((volatile u8*)global_obj + 5u)) = g;
  REG8((u32)((volatile u8*)global_obj + 6u)) = b;

  ((void (*)(u8, s32))0x8014e5a0u)(
      REG8((u32)((volatile u8*)REG32(0x1f800044u) + 0x29u)), 0x10u);
}
