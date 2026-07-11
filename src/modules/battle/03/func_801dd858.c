#include "internal.h"

/* @behavior submits one effect id selected from the current local work's halfword
 * at `0x2c` and the caller's byte index.
 * @source 0x801dd858 FUN_801dd858
 */
void func_801dd858(u32 arg0) {
  func_8015df18(
      BATTLE_EFFECT_TABLE_AFD0[BATTLE_LOCAL_HALF_2C(BATTLE_LOCAL_WORK_PTR) +
                               ((arg0 & 0xffu) * 3u)]);
}
