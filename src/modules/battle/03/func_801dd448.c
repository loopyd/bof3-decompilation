#include "internal.h"

/* does: reports whether the current local work is eligible for the queued
 * branch, optionally applying the `0x40` global gate and one random threshold.
 * @source: 0x801dd448 FUN_801dd448
 */
u8 func_801dd448(void) {
  if ((BATTLE_GLOBAL_HALF_62E8 & 0x40u) != 0u) {
    BATTLE_GLOBAL_HALF_62E8 &= 0xffbfu;
    return 0u;
  }

  if (((BATTLE_LOCAL_WORD_124(BATTLE_LOCAL_WORK_PTR) & 1u) == 0u) &&
      (BATTLE_GLOBAL_BYTE_6374 > 2u) &&
      ((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 0x4864u) == 0u) &&
      ((BATTLE_GLOBAL_BYTE_6375 != 4u) || (BATTLE_GLOBAL_HALF_63C0 != 0xa1u)) &&
      ((BATTLE_GLOBAL_BYTE_63CE == 0u) ||
       ((BATTLE_LOCAL_WORD_128(BATTLE_LOCAL_WORK_PTR) & 0x10u) != 0u))) {
    if ((BATTLE_LOCAL_WORD_124(BATTLE_LOCAL_WORK_PTR) & 0x8000u) == 0u) {
      return (func_8017e3d4() % 100) <
             BATTLE_LOCAL_BYTE_A9(BATTLE_LOCAL_WORK_PTR);
    }
    return 1u;
  }

  return 0u;
}
