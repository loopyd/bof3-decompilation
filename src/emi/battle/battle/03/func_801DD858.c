#include "internal.h"

/* @behavior submits one effect id selected from the current local work's halfword
 * at `0x2c` and the caller's byte index.
 * @source 0x801DD858
 */
void func_801DD858(u32 arg0) {
  u32 masked = arg0 & 0xffu;
  const volatile u16* table = D_801EAFD0;
  u32 index = masked * 3u;
  func_8015DF18((&table[index])[BATTLE_LOCAL_HALF_2C(BATTLE_LOCAL_WORK_PTR)]);
}
