#include "internal.h"

/* does: queues one AREA030 icon strip primitive, then optionally emits a small
 * red tile marker when `arg5` is set and the shared flag bit is still clear.
 * @source: 0x801d3244 FUN_801d3244
 */
void func_801d3244(s16 arg0, s16 arg1, u8 arg2, s8 arg3, u8 arg4, s8 arg5) {
  u32 primitive;
  u16 value;

  func_801e0c80(0, arg4);
  primitive = func_801e0dcc(0x1c, arg4, arg0, arg1);

  *(volatile u16*)(primitive + 0x10) = arg2;
  *(volatile u8*)(primitive + 0x0cu) =
      (u8)(*(volatile u8*)(primitive + 0x0cu) + arg3);

  if ((arg5 != 0) && ((WORLD00_AREA030_GLOBAL_WORD_3E6C & 8u) == 0u)) {
    func_801e0c80(6, 2);
    primitive = (u32)WORLD00_AREA030_PRIMITIVE_PTR;
    SetTile((TILE*)primitive);
    func_8017a904((void*)primitive, 1);
    *(volatile u16*)(primitive + 0x0eu) = 8u;
    *(volatile s16*)(primitive + 8) = arg0;
    *(volatile s16*)(primitive + 10) = arg1;
    *(volatile u16*)(primitive + 0x0cu) = arg2;
    *(volatile u8*)(primitive + 4) = 0xffu;
    *(volatile u8*)(primitive + 5) = 0u;
    *(volatile u8*)(primitive + 6) = 0u;
    func_8014e5a0(arg4, 0x10u);
  }
}
