#include "internal.h"

/* @behavior chooses one event id from the small fixed table based on the current
 * halfword mode at `0x801463d0`, then writes that event into slot `0`.
 * @source 0x801DEAE0
 */
void func_801DEAE0(void) {
  const void* src;
  u32*        mode_ptr;
  u8          local_18[6];
  u32         mode;
  u32         temp;
  u8*         table;

  src = (const void*)0x801d0000u;
  mode_ptr = (u32*)0x80140000u;
  __builtin_memcpy(local_18, (const u8*)src + 0xc98, sizeof(local_18));
  mode = *(volatile u32*)((u8*)mode_ptr + 0x63d0);

  if (mode == 1u) {
    temp = ((u32 (*)(void))func_8017E3D4)();
    table = local_18;
  } else {
    if ((mode == 0u) || (mode >= 5u)) {
      table = &local_18[4];
    } else {
      table = &local_18[2];
    }
    temp = ((u32 (*)(void))func_8017E3D4)();
  }
  func_801DE60C(0u, 2u, 0u, 0u, 0xffu, func_801502D0(table[temp & 1u]));
}
