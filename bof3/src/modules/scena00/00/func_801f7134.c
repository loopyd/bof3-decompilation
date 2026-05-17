#include "internal.h"

/* does: selects and positions a 3d model based on game progression state.
 * determines a colour palette id from the current chapter, configures a
 * sprite object with size/position/colour data from scratchpad tables,
 * and submits it for rendering.
 * @source: 0x801f7134 FUN_801f7134
 */
void func_801f7134(s32 chapter_id) {
  volatile void* global_obj;
  volatile u8*   scratchpad;
  s32            palette_code;

  global_obj = (volatile void*)REG32(0x8014598cu);

  palette_code = 0x32du;
  if (((s32(*)())0x8017b2b4u)() != 1) {
    palette_code = 0xddu;
    if (((s32(*)())0x8017b2b4u)() == 2) {
      palette_code = 0x32du;
    }
  }

  ((void(*)(volatile void*, s32, s32, s32, s32))0x8017c2d8u)(
      global_obj, 0, 0, palette_code, 0);

  scratchpad = (volatile u8*)REG32(0x1f800044u);
  ((void(*)(u8, s32))0x8014e5a0u)(REG8((u32)(scratchpad + 0x29u)), 0xcu);

  global_obj = (volatile void*)REG32(0x8014598cu);
  ((void(*)(volatile void*))0x8017aa1cu)(global_obj);

  REG16((u32)((volatile u8*)global_obj + 8u))  = 0x20;
  REG16((u32)((volatile u8*)global_obj + 0xau)) = 0x20;
  REG8((u32)((volatile u8*)global_obj + 0xcu))   = 0;
  REG8((u32)((volatile u8*)global_obj + 0xdu))   = 0x20;
  REG16((u32)((volatile u8*)global_obj + 0x10u)) = 0x100;
  REG16((u32)((volatile u8*)global_obj + 0x12u)) = 0x6e8;
  REG16((u32)((volatile u8*)global_obj + 0xeu))  = 0x7a80;

  scratchpad = (volatile u8*)REG32(0x1f800044u);
  REG8((u32)((volatile u8*)global_obj + 4u)) = REG8((u32)(scratchpad + 0x5du));
  REG8((u32)((volatile u8*)global_obj + 5u)) = REG8((u32)(scratchpad + 0x5eu));
  REG8((u32)((volatile u8*)global_obj + 6u)) = REG8((u32)(scratchpad + 0x5fu));

  ((void(*)(volatile void*, s32))0x8017a92cu)(global_obj, 0);
  ((void(*)(volatile void*, s32))0x8017a904u)(global_obj, chapter_id & 0xffu);

  scratchpad = (volatile u8*)REG32(0x1f800044u);
  ((void(*)(u8, s32))0x8014e5a0u)(REG8((u32)(scratchpad + 0x29u)), 0x14u);
}
