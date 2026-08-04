#include "internal.h"

/* @behavior reports whether the current enemy work is immediately ready, otherwise
 * delegating to the first EXE-side readiness helper.
 * @source 0x801E30F8
 */
u8 func_801E30F8(void) {
  volatile Battle03EnemyWork* enemy_work =
      PSX_REF(volatile Battle03EnemyWork*, 0x801EB4E8u);
  volatile Battle03EnemyWork* ew = enemy_work;

  if ((FIELD_REF(u16, ew, 0x82u) & 0x44u) != 0u) {
    return 1u;
  }
  if (BATTLE_GLOBAL_BYTE_63CE != 0u && (FIELD_REF(u32, ew, 0x104u) & 0x10u) == 0u) {
    return 1u;
  }
  return func_8014D978();
}