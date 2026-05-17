#include "internal.h"

/* does: allocates one primitive, assigns the rgb triple directly, stores two
 * endpoints, then queues it with mode `1`.
 * @source: 0x801da4b4 FUN_801da4b4
 */
void func_801da4b4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6) {
  u16 primitive_id;
  u32 primitive;

  primitive_id = func_8017a620(0, 0, 0x3c0, 0);
  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0, primitive_id, 0);
  func_8014e5a0(1u, 0x0cu);

  primitive = BATTLE_GLOBAL_WORD_598C;
  func_8017aa80(primitive);
  *(volatile u8*)(primitive + 4) = arg4;
  *(volatile u8*)(primitive + 5) = arg5;
  *(volatile u8*)(primitive + 6) = arg6;
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 10) = arg1;
  *(volatile s16*)(primitive + 12) = arg2;
  *(volatile s16*)(primitive + 14) = arg3;
  func_8017a904(primitive, 1);
  func_8014e5a0(1u, 0x10u);
}
